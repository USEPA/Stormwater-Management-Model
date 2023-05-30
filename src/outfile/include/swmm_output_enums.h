/*!
 *  \file swmm_output_enums.h
 *  \author Michael Tryby (US EPA - ORD/CESER)
 *  \author Caleb Buahin (US EPA - ORD/CESER) (Editor)
 *  \date Created On: 10/18/2019
 *  \date Last Updated: 05/18/2021
 *  \brief Header file for SWMM output enumeration types
 *  \remarks
 *  \see
 *  \bug
 *  \warning
 *  \todo
 *  \note
 */

 #ifndef SWMM_OUTPUT_ENUMS_H_
 #define SWMM_OUTPUT_ENUMS_H_

/*!
* \brief Unit system used in the simulation
*/
typedef enum {
     SMO_US, /*! \brief US Customary units (ft, acre, etc.)*/
     SMO_SI /*! \brief International System of Units (m, ha, etc.)*/
} SMO_unitSystem;

/*!
* \brief Flow units used in the simulation
*/
typedef enum {
     SMO_CFS, /*! \brief Cubic feet per second*/
     SMO_GPM, /*! \brief Gallons per minute*/
     SMO_MGD, /*! \brief Million gallons per minute*/
     SMO_CMS, /*! \brief Cubic meters per second*/
     SMO_LPS, /*! \brief Liters per second*/
     SMO_MLD /*! \brief Million liters per day*/
} SMO_flowUnits;

/*!
* \brief Concentration units used in the simulation
*/
typedef enum {
    SMO_MG, /*! \brief Milligrams per liter*/
    SMO_UG, /*! \brief Micrograms per liter*/
    SMO_COUNT,  /*! \brief Count per liter*/
    SMO_NONE /*! \brief No units*/
} SMO_concUnits;

/*!
* \brief SWMM Element types
*/
typedef enum {
    SMO_subcatch, /*! \brief SWMM Subcatchment*/
    SMO_node, /*! \brief SWMM Node*/
    SMO_link, /*! \brief SWMM Link*/
    SMO_sys, /*! \brief SWMM System*/
    SMO_pollut /*! \brief SWMM Pollutants*/
} SMO_elementType;

/*!
* \brief SWMM report time attributes
*/
typedef enum {
    SMO_reportStep, /*! \brief Report step size (seconds)*/
    SMO_reportStep, /*! \brief Report step size (seconds)*/
    SMO_numPeriods /*! \brief Number of reporting periods*/
} SMO_time;

/*!
* \brief SWMM Subcatchment attributes
*/
typedef enum {
    SMO_rainfall_subcatch,     /*! \brief Subcatchment rainfall in (in / hr or mm / hr) */
    SMO_snow_depth_subcatch,   /*! \brief Subcatchment snow depth in (in or mm) */
    SMO_evap_loss, 			   /*! \brief Subcatchment evaporation loss (in / hr or mm / hr)*/
    SMO_infil_loss,			   /*! \brief Subcatchment infiltration loss (in/hr or mm/hr) */
    SMO_runoff_rate,     	   /*! \brief Subcatchment runoff (flow units) */
    SMO_gwoutflow_rate,		   /*! \brief Subcatchment groundwater outflow (flow units) */
    SMO_gwtable_elev,    	   /*! \brief Subcatchment groundwater table elevation (ft or m) */
    SMO_soil_moisture,		   /*! \brief Subcatchment soil moisture content (-) */
    SMO_pollutant_conc_subcatch	/*! \brief Subcatchment pollutant concentration */
} SMO_subcatchAttribute;

/*!
* \brief SWMM Node attributes
*/
typedef enum {
    /*!
    * \brief Node depth in (ft or m)
	*/
    SMO_invert_depth,  
    /*!
    * \brief Node hydraulic head in (ft or m)
    * \note Hydraulic head is the sum of the invert depth and the water surface elevation
    * \sa SMO_invert_depth
	*/
    SMO_hydraulic_head,  
    /*!
    * \brief Node volume in (ft3 or m3)
    * \note Volume is the product of the node's area and its depth
    */
    SMO_stored_ponded_volume, 
    /*!
    * \brief Node lateral inflow rate in (flow units)
    * \note Lateral inflow is the sum of any external inflows (e.g., runoff, groundwater) plus any internal inflows (e.g., RDII, direct inflow)
	*/
    SMO_lateral_inflow, 
    /*!
    * \brief Node total inflow rate in (flow units)
    * \note Total inflow is the sum of the lateral inflow plus any upstream inflow
    * \sa SMO_lateral_inflow
    */
    SMO_total_inflow,
    /*!
    * \brief Node flooding losses (flow units)
    * \note Overflow is the flow rate that exceeds the node's capacity
	*/
    SMO_flooding_losses,
    /*!
    * \brief Node pollutant concentration
    */
    SMO_pollutant_conc_node 
} SMO_nodeAttribute;

/*!
* \brief SWMM Link attributes
*/
typedef enum {
    /*!
    * \brief Link flow rate in (flow units)
	*/
    SMO_flow_rate_link,      	// (flow units),
    /*!
    * \brief Link flow depth in (ft or m)
    * \note Flow depth is the depth of flow in the link's conduit
    * \sa SMO_flow_volume
    * \sa SMO_flow_velocity
    */
    SMO_flow_depth,     		// (ft or m),
    
    /*!
    * \brief Link flow velocity in (ft/s or m/s)
    * \note Flow velocity is the velocity of flow in the link's conduit
    */ 
    SMO_flow_velocity,

    /*!
    * \brief Link flow volume in (ft3 or m3)
    * \note Flow volume is the volume of flow in the link's conduit
    */
    SMO_flow_volume,

    /*!
    * \brief Link capacity in (fraction of conduit filled)
    * \note Capacity is the ratio of the flow depth to the maximum flow depth
    */  
    SMO_capacity, 

    /*!
    * \brief Link pollutant concentration
    * \note Pollutant concentration is the concentration of the first pollutant in the link's conduit
    */
    SMO_pollutant_conc_link  	// first pollutant,
} SMO_linkAttribute;

/*!
* \brief SWMM System attributes
*/
typedef enum {
    /*!
    * \brief System air temperature in (deg. F or deg. C)
	*/
    SMO_air_temp,              	// (deg. F or deg. C),
    /*!
    * \brief System rainfall in (in/hr or mm/hr)
    * \note Rainfall is the subcatchment area weighted total rainfall over the entire study area
	*/
    SMO_rainfall_system,        // (in/hr or mm/hr),
    
    /*!
    * \brief System snow depth in (in or mm)
    * \note Snow depth is the subcatchment area weighted total snow depth over the entire study area
    * \sa SMO_rainfall_system
    */
    SMO_snow_depth_system,      // (in or mm),

    /*!
    * \brief System evaporation loss in (in/hr or mm/hr)
    */
    SMO_evap_infil_loss,	  	// (in/hr or mm/hr),

    /*!
    * \brief System runoff flow in (flow units)
    */
    SMO_runoff_flow,           	// (flow units),
    
    /*!
    * \brief System dry weather inflow in (flow units)
	*/
    SMO_dry_weather_inflow,    	// (flow units),
    
    /*!
    * \brief System groundwater inflow in (flow units)
	*/
    SMO_groundwater_inflow,    	// (flow units),
    
    /*!
    * \brief System RDII inflow in (flow units)
    */
    SMO_RDII_inflow,           	// (flow units),
    
    /*!
    * \brief System direct inflow in (flow units)
	*/
    SMO_direct_inflow,         	// user defined (flow units),
    
    /*!
    * \brief System total lateral inflow in (flow units)
    */
    SMO_total_lateral_inflow,  	// (sum of variables 4 to 8) //(flow units),
    
    /*!
    * \brief System flooding losses in (flow units)
	*/
    SMO_flood_losses,       	// (flow units),
    
    /*!
    * \brief System outfall flow in (flow units)
    */
    SMO_outfall_flows,         	// (flow units),
    
    /*!
    * \brief System volume stored in (ft3 or m3)
	*/
    SMO_volume_stored,         	// (ft3 or m3),
    
    /*!
    * \brief System evaporation rate in (in/day or mm/day)
	*/
    SMO_evap_rate             	// (in/day or mm/day),
	//p_evap_rate		    // (in/day or mm/day)
} SMO_systemAttribute;


#endif /* SWMM_OUTPUT_ENUMS_H_ */
