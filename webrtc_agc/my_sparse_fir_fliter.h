#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>


typedef struct SparseFIRFilter
{
	size_t sparsity_;
	size_t offset_;

	float * nonzero_coeffs_;
	size_t nozero_coeffs_len_;
	float * state_;
	size_t state_len_;
};
SparseFIRFilter * SparseFIRFilter_Init(
	const float* nonzero_coeffs,
	size_t num_nonzero_coeffs,
	size_t sparsity,
	size_t offset) {

	if (num_nonzero_coeffs<1 || sparsity < 1)
	{
		return ;
	}
	SparseFIRFilter * handles = (SparseFIRFilter*)malloc(sizeof(SparseFIRFilter));


	handles->sparsity_ = sparsity;
	handles->offset_ = offset;

	handles->nozero_coeffs_len_ = num_nonzero_coeffs;
	handles->nonzero_coeffs_ = (float*)malloc( sizeof(float)*handles->nozero_coeffs_len_);
	memmove(handles->nonzero_coeffs_, nonzero_coeffs, handles->nozero_coeffs_len_);

	handles->state_len_ = handles->sparsity_ * (num_nonzero_coeffs - 1) + handles->offset_;
	handles->state_ = (float *)malloc(sizeof(float)*(handles->state_len_));
	memset(handles->state_, 0.0f, sizeof(float)*(handles->state_len_) );


	return handles;
}
void SparseFIRFilter_Filter(SparseFIRFilter *handles,const float* in, size_t length, float* out) {

	for (size_t i = 0; i < length; ++i) {
		out[i] = 0.f;
		size_t j;
		for (j = 0; i >= j * handles->sparsity_ + handles->offset_ && j < handles->nozero_coeffs_len_;
			++j) {
			out[i] += in[i - j * handles->sparsity_ - handles->offset_] * handles->nonzero_coeffs_[j];
		}
		for (; j < handles->nozero_coeffs_len_; ++j) {
			out[i] += handles->state_[i + (handles->nozero_coeffs_len_ - j - 1) * handles->sparsity_] *
				handles->nonzero_coeffs_[j];
		}
	}

	// Update current state.
	if (handles->state_len_ > 0u) {
		if (length >= handles->state_len_) {
			memcpy(handles->state_, &in[length - handles->state_len_],
				handles->state_len_ * sizeof(*in));
		}
		else {
			memmove(handles->state_, handles->state_+length,
				(handles->state_len_ - length) * sizeof(handles->state_[0]));
			memcpy( handles->state_ + handles->state_len_ - length , in, length * sizeof(*in));
		}
	}
}

int SparseFIRFilter_Destory() {
	return 0;
}