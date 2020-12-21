# OWA SWMM v5.1.13: Release Notes

OWA is a community lead effort to develop and maintain SWMM. As part of that
effort a number of new API calls have been added that aren't part of the
official USEPA SWMM release.

See the AUTHORS file for a list of contributors.


## Toolkit API (new functions):

### Projects
swmm_project_findObject()  
swmm_getObjectId()  
swmm_countObjects()  

### Simulation Settings
swmm_getSimulationDateTime()  
swmm_getCurrentDateTime()  
swmm_setSimulationDateTime()  
swmm_getSimulationAnalysisSetting()  
swmm_getSimulationParam()  
swmm_getSimulationUnit()  

### Nodes
swmm_getNodeType()  
swmm_getNodeParam()  
swmm_setNodeParam()  
swmm_getNodeResult()  
swmm_getNodePollut()  
swmm_getNodeTotalInflow()  
swmm_setNodeInflow()  
swmm_getNodeStats()  

### Storage
swmm_getStorageStats()  

### Outfall
swmm_setOutfallStage()  
swmm_getOutfallStats()  

### Links
swmm_getLinkType()  
swmm_getLinkConnections()  
swmm_getLinkDirection()  
swmm_getLinkParam()  
swmm_setLinkParam()  
swmm_getLinkResult()  
swmm_getLinkPollut()  
swmm_setLinkSetting()  
swmm_getLinkStats()  

### Pumps
swmm_getPumpStats()  

### Subcatchments
swmm_getSubcatchOutConnection()  
swmm_getSubcatchParam()  
swmm_setSubcatchParam()  
swmm_getSubcatchResult()  
swmm_getSubcatchPollut()  
swmm_getSubcatchStats()  

### System
swmm_getSystemRoutingTotals()  
swmm_getSystemRunoffTotals()  

### LIDs
swmm_getLidUCount()  
swmm_getLidUParam()  
swmm_setLidUParam()  
swmm_getLidUOption()  
swmm_setLidUOption()  
swmm_getLidUFluxRates()  
swmm_getLidUResult()  
swmm_getLidCOverflow()  
swmm_getLidCParam()  
swmm_setLidCParam()  
swmm_getLidGResult()  

### Rain Gages
swmm_getGagePrecip()  
swmm_setGagePrecip()  

### Utility
swmm_getVersionInfo()  
