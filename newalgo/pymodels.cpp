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

boost::python::object ConvertFeatureVector(const vector<float>& vec, bool hasFirstDim) {
	npy_intp size_f[2] = {1, vec.size()};
	npy_intp size_nf[1] = {vec.size()};
	npy_intp* size = hasFirstDim ? size_f : size_nf;

	PyArray_SimpleNew(1, size_nf, NPY_FLOAT);
	auto pyObj = PyArray_SimpleNew(1 + (int)hasFirstDim, size, NPY_FLOAT);
	float *buffer = (float *) PyArray_DATA(pyObj);
	std::memcpy(buffer, vec.data(), sizeof(float) * vec.size());
  // create a PyObject * from pointer and data
	boost::python::handle<> handle( pyObj );
	boost::python::numeric::array arr( handle );

  /* The problem of returning arr is twofold: firstly the user can modify
	the data which will betray the const-correctness
	Secondly the lifetime of the data is managed by the C++ API and not the
	lifetime of the numpy array whatsoever. But we have a simple solution..
   */

	 return arr.copy(); // copy the object. numpy owns the copy now.
}

boost::python::object ConvertLayeredFeatureVector(const vector<float>& vec, bool hasFirstDim) {
	PREF_ASSERT(vec.size() % 32 == 0);
	uint32_t layerCount = vec.size() / 32;
	npy_intp size[2] = {8, layerCount};

	bp::list res;
	for (uint32_t layer = 0; layer < 4; layer++) {
		auto pyObj = PyArray_SimpleNew(2 + (int)hasFirstDim, size, NPY_FLOAT);
		float *buffer = (float *) PyArray_DATA(pyObj);
		uint32_t counter = 0;
		for (uint32_t i = 0; i < vec.size(); i += 32) {
			for (uint32_t j = layer * 8; j < layer * 8 + 8; j++) {
				buffer[layerCount * j + i / 32] = vec[i + j];
			}
		}
	  // create a PyObject * from pointer and data
		bp::handle<> handle(pyObj);
		bp::numeric::array arr(handle);
		res.append(arr.copy());
	}
	return res;
}


class PyModel : public IModel {
public:
	PyModel(const std::string& modelName, FeatureTag modelType)
		: modelName_(modelName)
		, modelType_(modelType)
	{
		auto interp = GetInterpreter();
		try {
			obj_ = interp.createKerasModel(modelName, modelName);
			layered_ = bp::extract<bool>(bp::call_method<bp::object>(obj_.ptr(), "is_layered"));
		} catch (bp::error_already_set& ) {
			HandlePyException();
		}
	}

	std::vector<float> Predict(const FeaturesSet& features) override {
		auto& interp = GetInterpreter();
		try {
			bp::object arrayObj;
			if (layered_) {
				arrayObj = ConvertFeatureVector(features.GetFeatures(), /*hasFirstDim=*/true);
			} else {
				arrayObj = ConvertLayeredFeatureVector(features.GetFeatures(), /*hasFirstDim=*/true);
			}
			bp::object res = bp::call_method<bp::object>(obj_.ptr(), "predict", arrayObj);
			auto np_ret = reinterpret_cast<PyArrayObject*>(res.ptr());
			auto data = PyArray_DATA(np_ret);
			auto size = PyArray_SHAPE(np_ret);
			return vector<float>((float*)data, (float*)data + size[0]);
		} catch (bp::error_already_set& err) {
			HandlePyException();
			return vector<float>();
		}
	}

private:
	const std::string modelName_;
	FeatureTag modelType_;
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

std::shared_ptr<IModel> CreateModel(const std::string& modelName, FeatureTag tag) {
	return std::shared_ptr<IModel>(new PyModel(modelName, tag));
}

} // namespace PyModels

