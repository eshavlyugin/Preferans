/*
 * interpreter_context.h
 *
 *  Created on: Oct 30, 2016
 *      Author: zhenyok
 */

#ifndef PYWRAP_INTERPRETER_CONTEXT_H_
#define PYWRAP_INTERPRETER_CONTEXT_H_

#include "train_model.h"

namespace PyModels {

	void Init(int argc, char** argv);
	shared_ptr<IModel> CreateModel(const std::string& modelName, FeatureTag tag);

}

#endif /* PYWRAP_INTERPRETER_CONTEXT_H_ */
