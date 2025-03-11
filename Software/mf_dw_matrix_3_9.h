
#ifndef DW_MATRIX
  #define DW_MATRIX

  #if (ARDUINO >= 100)
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif
//
// The window matrix represents the smearing (or convolution) that results from time-domain windowing.
// Premultiplying the ideal model column vector by the window matrix would result in the dft column vector.
// ad_dewindow is the inverse of the window matrix.
//
// Premultiplying the dft column vector by d_winv results in an estimate of the model column vector.
//
// Inversion is performed at https://www.mathsisfun.com/algebra/matrix-calculator.html
//
// The initial and final DEWINDOW_BOUND values of the dft vector must be discarded to form the model vector.
//
// For REF_CYCLES = 9, the convolution matrix is:
//
// 1.0   0.35  0.0  
// 0.35  1.0   0.35
// 0.0   0.35  1.0
//
// And its inverse is:
//
// 1.16225165562914 -0.463576158940397 0.162251655629139 
// -0.463576158940397 1.32450331125828 -0.463576158940397 
// 0.162251655629139 -0.463576158940397 1.16225165562914 
// 
// These values are then divided by the degree of the matrix ( 3.0 ) so that the overall gain is unity.
//

#define DEWINDOW_BOUND 2

double ad_dewindow[ ( DEWINDOW_BOUND + 1) * ( DEWINDOW_BOUND + 1) ] = {

 0.38741721854305,  -0.154525386313466,  0.054083885209713,
-0.154525386313466,  0.44150110375276,  -0.154525386313466,
 0.054083885209713, -0.154525386313466,  0.38741721854305
  
};

double* pad_dewindow = ad_dewindow;

#endif
