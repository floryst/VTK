/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageMedian3D.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
#include "vtkImageMedian3D.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageMedian3D* vtkImageMedian3D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageMedian3D");
  if(ret)
    {
    return (vtkImageMedian3D*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageMedian3D;
}






//----------------------------------------------------------------------------
// Construct an instance of vtkImageMedian3D fitler.
vtkImageMedian3D::vtkImageMedian3D()
{
  this->SetKernelSize(1,1,1);
  this->HandleBoundaries = 1;
}

//----------------------------------------------------------------------------
void vtkImageMedian3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageSpatialFilter::PrintSelf(os, indent);

  os << indent << "NumberOfElements: " << this->NumberOfElements << endl;
}

//----------------------------------------------------------------------------
// This method sets the size of the neighborhood.  It also sets the 
// default middle of the neighborhood 
void vtkImageMedian3D::SetKernelSize(int size0, int size1, int size2)
{  
  int volume;
  int modified = 1;
  
  if (this->KernelSize[0] == size0 && this->KernelSize[1] == size1 && 
      this->KernelSize[2] == size2)
    {
  	modified = 0;
    }
  
  // Set the kernel size and middle
  volume = 1;
  this->KernelSize[0] = size0;
  this->KernelMiddle[0] = size0 / 2;
  volume *= size0;
  this->KernelSize[1] = size1;
  this->KernelMiddle[1] = size1 / 2;
  volume *= size1;
  this->KernelSize[2] = size2;
  this->KernelMiddle[2] = size2 / 2;
  volume *= size2;

  this->NumberOfElements = volume;
  if ( modified )
  {
	  this->Modified();
  }
}

//----------------------------------------------------------------------------
// Add a sample to the median computation
double *vtkImageMedian3DAccumulateMedian(int &UpNum, int &DownNum,
					 int &UpMax, int &DownMax,
					 int &NumNeighborhood,
					 double *Median, double val)
{
  int idx, max;
  double temp, *ptr;

  // special case: no samples yet 
  if (UpNum == 0)
    {
    *(Median) = val;
    // length of up and down arrays inclusive of current 
    UpNum = DownNum = 1; 
    // median is gaurenteed to be in this range (length of array) 
    DownMax = UpMax = (NumNeighborhood + 1) / 2;
    return Median;
    }

  // Case: value is above median 
  if (val >= *(Median))
    {
    // move the median if necessary
    if (UpNum > DownNum)
      {
      // Move the median Up one 
      ++Median;
      --UpNum;
      ++DownNum;
      --UpMax;
      ++DownMax;
      }
    // find the position for val in the sorted array
    max = (UpNum < UpMax) ? UpNum : UpMax;
    ptr = Median;
    idx = 0;
    while (idx < max && val >= *ptr)
      {
      ++ptr;
      ++idx;
      }
    // place val and move all others up
    while (idx <= max)
      {
      temp = *ptr;
      *ptr = val;
      val = temp;
      ++ptr;
      ++idx;
      }
    // Update counts
    ++UpNum;
    --DownMax;
    return Median;
    }


  // Case: value is below median 
  if (val <= *(Median))
    {
    // move the median if necessary
    if (DownNum > UpNum)
      {
      // Move the median Down one 
      --Median;
      --DownNum;
      ++UpNum;
      --DownMax;
      ++UpMax;
      }
    // find the position for val in the sorted array
    max = (DownNum < DownMax) ? DownNum : DownMax;
    ptr = Median;
    idx = 0;
    while (idx < max && val <= *ptr)
      {
      --ptr;
      ++idx;
      }
    // place val and move all others up
    while (idx <= max)
      {
      temp = *ptr;
      *ptr = val;
      val = temp;
      --ptr;
      ++idx;
      }
    // Update counts
    ++DownNum;
    --UpMax;
    return Median;
    }
  return Median;
}


//----------------------------------------------------------------------------
// This method contains the second switch statement that calls the correct
// templated function for the mask types.
template <class T>
static void vtkImageMedian3DExecute(vtkImageMedian3D *self,
				    vtkImageData *inData, T *inPtr, 
				    vtkImageData *outData, T *outPtr,
				    int outExt[6], int id)
{
  int *kernelMiddle, *kernelSize;
  int NumberOfElements;
  // For looping though output (and input) pixels.
  int outIdx0, outIdx1, outIdx2;
  int inInc0, inInc1, inInc2;
  int outIdxC, outIncX, outIncY, outIncZ;
  T *inPtr0, *inPtr1, *inPtr2;
  // For looping through hood pixels
  int hoodMin0, hoodMax0, hoodMin1, hoodMax1, hoodMin2, hoodMax2;
  int hoodStartMin0, hoodStartMax0, hoodStartMin1, hoodStartMax1;
  int hoodIdx0, hoodIdx1, hoodIdx2;
  T *tmpPtr0, *tmpPtr1, *tmpPtr2;
  // The portion of the out image that needs no boundary processing.
  int middleMin0, middleMax0, middleMin1, middleMax1, middleMin2, middleMax2;
  int numComp;
  // variables for the median calc
  int UpNum, DownNum, UpMax, DownMax;
  double *Median;
  double *Sort = new double[(self->GetNumberOfElements() + 8)];
  int *inExt;
  unsigned long count = 0;
  unsigned long target;
  
  // Get information to march through data
  inData->GetIncrements(inInc0, inInc1, inInc2); 
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  kernelMiddle = self->GetKernelMiddle();
  kernelSize = self->GetKernelSize();
  
  numComp = inData->GetNumberOfScalarComponents();

  hoodMin0 = outExt[0] - kernelMiddle[0]; 
  hoodMin1 = outExt[2] - kernelMiddle[1]; 
  hoodMin2 = outExt[4] - kernelMiddle[2]; 
  hoodMax0 = kernelSize[0] + hoodMin0 - 1;
  hoodMax1 = kernelSize[1] + hoodMin1 - 1;
  hoodMax2 = kernelSize[2] + hoodMin2 - 1;
  
  // Clip by the input image extent
  inExt = inData->GetExtent();
  hoodMin0 = (hoodMin0 > inExt[0]) ? hoodMin0 : inExt[0];
  hoodMin1 = (hoodMin1 > inExt[2]) ? hoodMin1 : inExt[2];
  hoodMin2 = (hoodMin2 > inExt[4]) ? hoodMin2 : inExt[4];
  hoodMax0 = (hoodMax0 < inExt[1]) ? hoodMax0 : inExt[1];
  hoodMax1 = (hoodMax1 < inExt[3]) ? hoodMax1 : inExt[3];
  hoodMax2 = (hoodMax2 < inExt[5]) ? hoodMax2 : inExt[5];

  // Save the starting neighborhood dimensions (2 loops only once)
  hoodStartMin0 = hoodMin0;    hoodStartMax0 = hoodMax0;
  hoodStartMin1 = hoodMin1;    hoodStartMax1 = hoodMax1;
  
  // The portion of the output that needs no boundary computation.
  middleMin0 = inExt[0] + kernelMiddle[0];
  middleMax0 = inExt[1] - (kernelSize[0] - 1) + kernelMiddle[0];
  middleMin1 = inExt[2] + kernelMiddle[1];
  middleMax1 = inExt[3] - (kernelSize[1] - 1) + kernelMiddle[1];
  middleMin2 = inExt[4] + kernelMiddle[2];
  middleMax2 = inExt[5] - (kernelSize[2] - 1) + kernelMiddle[2];
  
  target = (unsigned long)((outExt[5] - outExt[4] + 1)*
    (outExt[3] - outExt[2] + 1)/50.0);
  target++;
  
  NumberOfElements = self->GetNumberOfElements();

  // loop through pixel of output
  inPtr = (T *)inData->GetScalarPointer(hoodMin0,hoodMin1,hoodMin2);
  inPtr2 = inPtr;
  for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
    {
    inPtr1 = inPtr2;
    hoodMin1 = hoodStartMin1;
    hoodMax1 = hoodStartMax1;
    for (outIdx1 = outExt[2]; 
	 !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1)
      {
      if (!id) 
	{
	if (!(count%target))
	  {
	  self->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      inPtr0 = inPtr1;
      hoodMin0 = hoodStartMin0;
      hoodMax0 = hoodStartMax0;
      for (outIdx0 = outExt[0]; outIdx0 <= outExt[1]; ++outIdx0)
	{
	for (outIdxC = 0; outIdxC < numComp; outIdxC++)
	  {
	  // Compute median of neighborhood
	  // Note: For boundary, NumNeighborhood could be changed for
	  // a faster sort.
	  DownNum = UpNum = 0;
          Median = Sort + (NumberOfElements / 2) + 4;
	  // loop through neighborhood pixels
	  tmpPtr2 = inPtr0 + outIdxC;
	  for (hoodIdx2 = hoodMin2; hoodIdx2 <= hoodMax2; ++hoodIdx2)
	    {
	    tmpPtr1 = tmpPtr2;
	    for (hoodIdx1 = hoodMin1; hoodIdx1 <= hoodMax1; ++hoodIdx1)
	      {
	      tmpPtr0 = tmpPtr1;
	      for (hoodIdx0 = hoodMin0; hoodIdx0 <= hoodMax0; ++hoodIdx0)
		{
		// Add this pixel to the median
		Median = vtkImageMedian3DAccumulateMedian(UpNum, DownNum, 
							  UpMax, DownMax,
							  NumberOfElements,
							  Median,
							  double(*tmpPtr0));
		
		tmpPtr0 += inInc0;
		}
	      tmpPtr1 += inInc1;
	      }
	    tmpPtr2 += inInc2;
	    }
	
	  // Replace this pixel with the hood median
	  *outPtr = (T)(*Median);
	  outPtr++;
	  }
	
	// shift neighborhood considering boundaries
	if (outIdx0 >= middleMin0)
	  {
	  inPtr0 += inInc0;
	  ++hoodMin0;
	  }
	if (outIdx0 < middleMax0)
	  {
	  ++hoodMax0;
	  }
	}
      // shift neighborhood considering boundaries
      if (outIdx1 >= middleMin1)
	{
	inPtr1 += inInc1;
	++hoodMin1;
	}
      if (outIdx1 < middleMax1)
	{
	++hoodMax1;
	}
      outPtr += outIncY;
    }
    // shift neighborhood considering boundaries
    if (outIdx2 >= middleMin2)
      {
      inPtr2 += inInc2;
      ++hoodMin2;
      }
    if (outIdx2 < middleMax2)
      {
      ++hoodMax2;
      }
    outPtr += outIncZ;
    }

  delete [] Sort;
  
}

//----------------------------------------------------------------------------
// This method contains the first switch statement that calls the correct
// templated function for the input and output region types.
void vtkImageMedian3D::ThreadedExecute(vtkImageData *inData, 
				       vtkImageData *outData,
				       int outExt[6], int id)
{
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
  << ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
                  << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageMedian3DExecute(this, inData, (double *)(inPtr), 
			      outData, (double *)(outPtr),outExt, id);
      break;
    case VTK_FLOAT:
      vtkImageMedian3DExecute(this, inData, (float *)(inPtr), 
			      outData, (float *)(outPtr),outExt, id);
      break;
    case VTK_LONG:
      vtkImageMedian3DExecute(this, inData, (long *)(inPtr), 
			      outData, (long *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageMedian3DExecute(this, inData, (unsigned long *)(inPtr), 
			      outData, (unsigned long *)(outPtr),outExt, id);
      break;
    case VTK_INT:
      vtkImageMedian3DExecute(this, inData, (int *)(inPtr), 
			      outData, (int *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageMedian3DExecute(this, inData, (unsigned int *)(inPtr), 
			      outData, (unsigned int *)(outPtr),outExt, id);
      break;
    case VTK_SHORT:
      vtkImageMedian3DExecute(this, inData, (short *)(inPtr), 
			      outData, (short *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageMedian3DExecute(this, inData, (unsigned short *)(inPtr), 
			      outData, (unsigned short *)(outPtr),outExt, id);
      break;
    case VTK_CHAR:
      vtkImageMedian3DExecute(this, inData, (char *)(inPtr), 
			      outData, (char *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageMedian3DExecute(this, inData, (unsigned char *)(inPtr), 
			      outData, (unsigned char *)(outPtr),outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown input ScalarType");
      return;
    }
}


