/*
 * interpreter_context.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: zhenyok
 */

#include "pymodels.h"
#include "features.h"

#include <Python.h>
#include <boost/python/numeric.hpp>

namespace bp = boost::python;

namespace PyModels {

class PyInterpreter {
public:
	PyInterpreter() {
		main_module_ = bp::import("__main__");
		main_namespace_ = main_module_.attr("__dict__");
		bp::exec_file("keras_models.py", main_namespace_, main_namespace_);
	}

	bp::object eval(const std::string& cmd) {
		bp::eval(bp::str(cmd), main_namespace_, main_namespace_);
	}

	bp::object createKerasModel(const std::string& objName, const std::string& modelFile) {
		std::string pyCmd = "KerasModel(\"" + modelFile + "\")";
		return bp::eval(bp::str(pyCmd), main_namespace_, main_namespace_);
	}

private:
	bp::object main_module_;
	bp::object main_namespace_;
};

PyInterpreter& GetInterpreter() {
	static PyInterpreter interp;
	return interp;
}

static void HandlePyException() {
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	bp::handle<> hType(ptype);
	bp::object extype(hType);
	//bp::handle<> hTraceback(ptraceback);
	//bp::object traceback(hTraceback);
	//Extract error message
	string strErrorMessage = bp::extract<string>(pvalue);
	cerr << strErrorMessage  << endl;
	//Extract line number (top entry of call stack)
	// if you want to extract another levels of call stack
	// also process traceback.attr("tb_next") recurently
	//long lineno = bp::extract<long> (traceback.attr("tb_lineno"));
	//string filename = bp::extract<string>(traceback.attr("tb_frame").attr("f_code").attr("co_filename"));
	//string funcname = bp::extract<string>(traceback.attr("tb_frame").attr("f_code").attr("co_name"));
	//cerr << strErrorMessage << "\n" << filename << ":" << lineno << "\n" << funcname << endl;
}

class PyModel : public IModel {
public:
	PyModel(const std::string& modelName)
		: modelName_(modelName)
	{
		auto interp = GetInterpreter();
		try {
			obj_ = interp.createKerasModel(modelName, modelName);
			layered_ = bp::extract<bool>(bp::call_method<bp::object>(obj_.ptr(), "is_layered"));
		} catch (bp::error_already_set& ) {
			HandlePyException();
		}
	}

	std::vector<float> PredictSeq(const vector<FeaturesSet>& featuresSeries) override {
		auto& interp = GetInterpreter();
		try {
			vector<float> features;
			for (const auto& featureObj : featuresSeries) {
				auto floatVec = featureObj.GetFeatures();
				features.insert(features.end(), floatVec.begin(), floatVec.end());
			}
			bp::object arrayObj;
			if (layered_) {
				arrayObj = ConvertLayeredSeqFeatureVector(features, featuresSeries.size());
			} else {
				arrayObj = ConvertSeqFeatureVector(features, featuresSeries.size());
			}
			bp::object res = bp::call_method<bp::object>(obj_.ptr(), "predict", arrayObj);
			auto np_ret = reinterpret_cast<PyArrayObject*>(res.ptr());
			auto data = (float*)PyArray_DATA(np_ret);
			auto size = PyArray_SHAPE(np_ret);
			return vector<float>(data, data + size[1]);
		} catch (bp::error_already_set& err) {
			HandlePyException();
			return vector<float>();
		}
	}

	std::vector<float> Predict(const FeaturesSet& features) override {
		auto& interp = GetInterpreter();
		try {
			bp::object arrayObj;
			if (layered_) {
				arrayObj = ConvertLayeredFeatureVector(features.GetFeatures(), /*hasFirstDim=*/true);
			} else {
				arrayObj = ConvertFeatureVector(features.GetFeatures(), /*hasFirstDim=*/true);
			}
			bp::object res = bp::call_method<bp::object>(obj_.ptr(), "predict", arrayObj);
			auto np_ret = reinterpret_cast<PyArrayObject*>(res.ptr());
			auto data = (float*)PyArray_DATA(np_ret);
			auto size = PyArray_SHAPE(np_ret);
			return vector<float>(data, data + size[1]);
		} catch (bp::error_already_set& err) {
			HandlePyException();
			return vector<float>();
		}
	}

private:
	const std::string modelName_;
	bp::object obj_;
	bool layered_ = false;
};

void Init(int argc, char** argv) {
	try {
	    Py_Initialize();
		PySys_SetArgv(argc, argv);
	    GetInterpreter();
	    import_array()
	    bp::numeric::array::set_module_and_type("numpy", "ndarray");
	}catch(bp::error_already_set& err){
		HandlePyException();
	}
}

std::shared_ptr<IModel> PyModelFactory::CreateModel(const std::string& modelName) {
	return shared_ptr<IModel>(new PyModel(modelName));
}

} // namespace PyModels

