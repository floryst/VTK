/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageReslice.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to David G. Gobbi who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkImageReslice - Reslices a volume along the axes specified.
// .SECTION Description
// vtkImageReslice will regrid a volume along the axes specified by
// the reslicing matrix.  The extent, origin, and sampling
// density of the output data can also be set.  This class is the
// swiss-army-knife of image geometry filters:  It can permute, flip,
// rotate, scale, resample, and pad image data in any combination. 
// It does the permute and resample operations at close to the 
// efficency of vtkImagePermute and vtkImageResample.  It can also
// extract oblique slices from image volumes, which no other VTK
// imaging filter can do.
// .SECTION Caveats
// This filter is very inefficient if the output X dimension is 1.
// .SECTION see also
// vtkImageToImageFilter, vtkTransform


#ifndef __vtkImageReslice_h
#define __vtkImageReslice_h


#include "vtkImageToImageFilter.h"
#include "vtkGeneralTransform.h"
#include "vtkMatrix4x4.h"

class vtkMatrix4x4;

#define VTK_RESLICE_NEAREST 0
#define VTK_RESLICE_LINEAR 1
#define VTK_RESLICE_CUBIC 3

class VTK_EXPORT vtkImageReslice : public vtkImageToImageFilter
{
public:
  static vtkImageReslice *New();
  vtkTypeMacro(vtkImageReslice,vtkImageToImageFilter);

  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the axes of the mesh along which the volume will be resliced. 
  // The axes are extracted from the 4x4 matrix:  The x-axis is the 
  // first column, the y-axis is the second column, the z-axis is the  
  // third column, and the origin is the final column.  The bottom 
  // row of the matrix should always be (0,0,0,1). 
  // If you don't set the axes, the axes will default to 
  // (1,0,0), (0,1,0), (0,0,1) and their origin will be (0,0,0). 
  // Generally, this is a permutation matrix (though it need not be). 
  vtkSetObjectMacro(ResliceAxes,vtkMatrix4x4);
  vtkGetObjectMacro(ResliceAxes,vtkMatrix4x4);

  // Description:
  // Set a transform to be applied to the reslicing axes.
  // If you don't set this, it will be treated as the identity transform.
  // This is often used to obtain oblique slices from the original data,
  // or to regrid one data set to match another given a linear
  // registration transformation.
  vtkSetObjectMacro(ResliceTransform,vtkGeneralTransform);
  vtkGetObjectMacro(ResliceTransform,vtkGeneralTransform);

  // Description:
  // Turn on wrap-pad feature (default: off). 
  vtkSetMacro(Wrap,int);
  vtkGetMacro(Wrap,int);
  vtkBooleanMacro(Wrap,int);

  // Description:
  // Turn on mirror-pad feature (default: off). 
  // This will override the wrap-pad, if set.
  vtkSetMacro(Mirror,int);
  vtkGetMacro(Mirror,int);
  vtkBooleanMacro(Mirror,int);

  // Description:
  // Set interpolation mode (default: nearest neighbor). 
  vtkSetMacro(InterpolationMode,int);
  vtkGetMacro(InterpolationMode,int);
  void SetInterpolationModeToNearestNeighbor()
    { this->SetInterpolationMode(VTK_RESLICE_NEAREST); };
  void SetInterpolationModeToLinear()
    { this->SetInterpolationMode(VTK_RESLICE_LINEAR); };
  void SetInterpolationModeToCubic()
    { this->SetInterpolationMode(VTK_RESLICE_CUBIC); };
  char *GetInterpolationModeAsString();

  // Description:
  // Obsolete method, but still convenient for switching between 
  // nearest-neighbor and linear interpolation (default: off). 
  void SetInterpolate(int terp) { this->SetInterpolationMode( \
     (terp ? VTK_RESLICE_LINEAR : VTK_RESLICE_NEAREST)); };
  void InterpolateOn() { this->SetInterpolationModeToLinear(); };
  void InterpolateOff() { this->SetInterpolationModeToNearestNeighbor(); };
  int GetInterpolate() { 
    return (this->GetInterpolationMode() != VTK_RESLICE_NEAREST); };

  // Description:
  // Turn on and off optimizations (default on, turn them off only if
  // they are not stable on your architecture). 
  vtkSetMacro(Optimization,int);
  vtkGetMacro(Optimization,int);
  vtkBooleanMacro(Optimization,int);

  // Description:
  // Set the background color (for multi-component images)
  vtkSetVector4Macro(BackgroundColor, float);
  vtkGetVector4Macro(BackgroundColor, float);

  // Description:
  // Set background grey level (for single-component images)
  void SetBackgroundLevel(float v) {this->SetBackgroundColor(v,v,v,v);};
  float GetBackgroundLevel() { return this->GetBackgroundColor()[0]; };

  // Description:
  // Spacing, origin, and extent of output data. 
  // The OutputSpacing default is (1,1,1). 
  // The OutputOrigin and OutputExtent are set to cover the entire
  // transformed input extent by default.
  vtkSetVector3Macro(OutputSpacing, float);
  vtkGetVector3Macro(OutputSpacing, float);
  vtkSetVector3Macro(OutputOrigin, float);
  vtkGetVector3Macro(OutputOrigin, float);
  vtkSetVector6Macro(OutputExtent, int);
  vtkGetVector6Macro(OutputExtent, int);

  // Description:
  // When determining the modified time of the filter, 
  // check the modified time of the transform.
  unsigned long int GetMTime();

  // Description:
  // Helper functions not meant to be used outside this class. 
//BTX
  vtkMatrix4x4 *GetIndexMatrix();
  int FindExtent(int& r1, int& r2, double *point, double *xAxis,
		      int *inMin, int *inMax, int *outExt);
//ETX
protected:
  vtkImageReslice();
  ~vtkImageReslice();
  vtkImageReslice(const vtkImageReslice&) {};
  void operator=(const vtkImageReslice&) {};

  vtkMatrix4x4 *ResliceAxes;
  vtkGeneralTransform *ResliceTransform;
  vtkMatrix4x4 *IndexMatrix;
  int Wrap;
  int Mirror;
  int InterpolationMode;
  int Optimization;
  float BackgroundColor[4];
  float OutputOrigin[3];
  float OutputSpacing[3];
  int OutputExtent[6];

  void ExecuteInformation(vtkImageData *input, vtkImageData *output);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int ext[6], int id);
  void OptimizedComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  void OptimizedThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
				int ext[6], int id);  
};

//----------------------------------------------------------------------------
inline char *vtkImageReslice::GetInterpolationModeAsString()
{
  switch (this->InterpolationMode)
    {
    case VTK_RESLICE_NEAREST:
      return "NearestNeighbor";
    case VTK_RESLICE_LINEAR:
      return "Linear";
    case VTK_RESLICE_CUBIC:
      return "Cubic";
    default:
      return "";
    }
}  

#endif





