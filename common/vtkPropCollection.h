/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPropCollection.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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
// .NAME vtkPropCollection - a list of Props
// .SECTION Description
// vtkPropCollection represents and provides methods to manipulate a list of
// Props (i.e., vtkProp and subclasses). The list is unsorted and duplicate
// entries are not prevented.

// .SECTION see also
// vtkProp vtkCollection 

#ifndef __vtkPropC_h
#define __vtkPropC_h

#include "vtkCollection.h"
class vtkProp;

class VTK_EXPORT vtkPropCollection : public vtkCollection
{
 public:
  static vtkPropCollection *New();
  vtkTypeMacro(vtkPropCollection,vtkCollection);

  // Description:
  // Add an Prop to the list.
  void AddItem(vtkProp *a);

  // Description:
  // Get the next Prop in the list.
  vtkProp *GetNextProp();

  // Description:
  // Get the last Prop in the list.
  vtkProp *GetLastProp();
  
protected:
  vtkPropCollection() {};
  ~vtkPropCollection() {};
  vtkPropCollection(const vtkPropCollection&) {};
  void operator=(const vtkPropCollection&) {};
  

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };

};

inline void vtkPropCollection::AddItem(vtkProp *a) 
{
  this->vtkCollection::AddItem((vtkObject *)a);
}

inline vtkProp *vtkPropCollection::GetNextProp() 
{ 
  return (vtkProp *)(this->GetNextItemAsObject());
}

inline vtkProp *vtkPropCollection::GetLastProp() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return (vtkProp *)(this->Bottom->Item);
    }
}

#endif





