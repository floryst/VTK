vtk_module(vtkFiltersGeneral
  GROUPS
    StandAlone
  TEST_DEPENDS
    vtkFiltersFlowPaths
    vtkFiltersModeling
    vtkIOGeometry
    vtkIOLegacy
    vtkIOXML
    vtkImagingMath
    vtkInteractionStyle
    vtkRenderingOpenGL2
    vtkRenderingAnnotation
    vtkRenderingLabel
    vtkTestingRendering
  KIT
    vtkFilters
  DEPENDS
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkCommonMisc
    vtkFiltersCore
  PRIVATE_DEPENDS
    vtkCommonComputationalGeometry
    vtkCommonMath
    vtkCommonSystem
    vtkCommonTransforms
  )
