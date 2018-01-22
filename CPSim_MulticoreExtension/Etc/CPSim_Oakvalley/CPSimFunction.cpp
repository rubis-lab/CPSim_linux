/**
  *@File    CPSimFunction.cpp
  *@Author  won.seok.django@gmail.com
  *@Brief   function definitions for CPSimTask
  */
#include "CPSimFunction.hpp"

#define MATRIX_SIZE 64

static unsigned int g_mat_Tau1_A[MATRIX_SIZE][MATRIX_SIZE] = { 0, };
static unsigned int g_mat_Tau1_B[MATRIX_SIZE][MATRIX_SIZE] = { 0, };
static unsigned int g_mat_Tau1_C[MATRIX_SIZE][MATRIX_SIZE] = { 0, };

void Tau1_func(void* _arg) {
  int row, col, idx;
  for (row = 0; row < MATRIX_SIZE; ++row) {
    for (col = 0; col < MATRIX_SIZE; ++col) {
      g_mat_Tau1_C[row][col] = 0;
      for (idx = 0; idx < MATRIX_SIZE; ++idx) {
        g_mat_Tau1_C[row][col] += g_mat_Tau1_A[row][idx] * g_mat_Tau1_B[idx][col];
      }
    }
  }
}

static unsigned int g_mat_Tau2_A[MATRIX_SIZE][MATRIX_SIZE] = { 0, };
static unsigned int g_mat_Tau2_B[MATRIX_SIZE][MATRIX_SIZE] = { 0, };
static unsigned int g_mat_Tau2_C[MATRIX_SIZE][MATRIX_SIZE] = { 0, };

void Tau2_func(void* _arg) {
  int row, col, idx;
  for (row = 0; row < MATRIX_SIZE; ++row) {
    for (col = 0; col < MATRIX_SIZE; ++col) {
      g_mat_Tau2_C[row][col] = 0;
      for (idx = 0; idx < MATRIX_SIZE; ++idx) {
        g_mat_Tau2_C[row][col] += g_mat_Tau2_A[row][idx] * g_mat_Tau2_B[idx][col];
      }
    }
  }
}
