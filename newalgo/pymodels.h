/*
 * interpreter_context.h
 *
 *  Created on: Oct 30, 2016
 *      Author: zhenyok
 */

#ifndef PYWRAP_INTERPRETER_CONTEXT_H_
#define PYWRAP_INTERPRETER_CONTEXT_H_

#include "train_model.h"

#include <boost/python.hpp>
#include <numpy/ndarrayobject.h>

namespace PyModels {

	void Init(int argc, char** argv);
	shared_ptr<IModel> CreateModel(const std::string& modelName, FeatureTag tag);

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

		 return arr.copy(); // copy the object. numpy owns the copy now.
	}

	boost::python::object ConvertLayeredFeatureVector(const vector<float>& vec, bool hasFirstDim) {
		PREF_ASSERT(vec.size() % 32 == 0);
		uint32_t layerCount = vec.size() / 32;
		npy_intp size[2] = {8, layerCount};

		boost::python::list res;
		for (uint32_t layer = 0; layer < 4; layer++) {
			auto pyObj = PyArray_SimpleNew(2 + (int)hasFirstDim, size, NPY_FLOAT);
			float *buffer = (float *) PyArray_DATA(pyObj);
			uint32_t counter = 0;
			for (uint32_t i = 0; i < vec.size(); i += 32) {
				for (uint32_t j = 0; j < 8; j++) {
					buffer[layerCount * j + i / 32] = vec[i + j + layer * 8];
				}
			}
		  // create a PyObject * from pointer and data
			boost::python::handle<> handle(pyObj);
			boost::python::numeric::array arr(handle);
			res.append(arr.copy());
		}
		return res;
	}

}

#endif /* PYWRAP_INTERPRETER_CONTEXT_H_ */
