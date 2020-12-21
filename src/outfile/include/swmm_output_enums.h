/*
 *  swmm_output_enums.h
 *
 *  Created on: October 18, 2019
 *
 *  Author: Michael E. Tryby
 *          US EPA - ORD/CESER
 */


 #ifndef SWMM_OUTPUT_ENUMS_H_
 #define SWMM_OUTPUT_ENUMS_H_


typedef enum {
     SMO_US,
     SMO_SI
} SMO_unitSystem;

typedef enum {
     SMO_CFS,
     SMO_GPM,
     SMO_MGD,
     SMO_CMS,
     SMO_LPS,
     SMO_MLD
} SMO_flowUnits;

typedef enum {
    SMO_MG,
    SMO_UG,
    SMO_COUNT,
    SMO_NONE
} SMO_concUnits;

typedef enum {
    SMO_subcatch,
    SMO_node,
    SMO_link,
    SMO_sys,
    SMO_pollut
} SMO_elementType;

typedef enum {
    SMO_reportStep,
    SMO_numPeriods
} SMO_time;

typedef enum {
    SMO_rainfall_subcatch,      // (in/hr or mm/hr),
    SMO_snow_depth_subcatch,    // (in or mm),
    SMO_evap_loss,              // (in/hr or mm/hr),
    SMO_infil_loss,             // (in/hr or mm/hr),
    SMO_runoff_rate,            // (flow units),
    SMO_gwoutflow_rate,         // (flow units),
    SMO_gwtable_elev,           // (ft or m),
    SMO_soil_moisture,          // unsaturated zone moisture content (-),
    SMO_pollutant_conc_subcatch	// first pollutant
} SMO_subcatchAttribute;

typedef enum {
    SMO_invert_depth,           // (ft or m),
    SMO_hydraulic_head,         // (ft or m),
    SMO_stored_ponded_volume,   // (ft3 or m3),
    SMO_lateral_inflow,         // (flow units),
    SMO_total_inflow,           // lateral + upstream (flow units),
    SMO_flooding_losses,        // (flow units),
    SMO_pollutant_conc_node     // first pollutant,
} SMO_nodeAttribute;

typedef enum {
    SMO_flow_rate_link,         // (flow units),
    SMO_flow_depth,             // (ft or m),
    SMO_flow_velocity,          // (ft/s or m/s),
    SMO_flow_volume,            // (ft3 or m3),
    SMO_capacity,               // (fraction of conduit filled),
    SMO_pollutant_conc_link     // first pollutant,
} SMO_linkAttribute;

typedef enum {
    SMO_air_temp,               // (deg. F or deg. C),
    SMO_rainfall_system,        // (in/hr or mm/hr),
    SMO_snow_depth_system,      // (in or mm),
    SMO_evap_infil_loss,        // (in/hr or mm/hr),
    SMO_runoff_flow,            // (flow units),
    SMO_dry_weather_inflow,     // (flow units),
    SMO_groundwater_inflow,     // (flow units),
    SMO_RDII_inflow,            // (flow units),
    SMO_direct_inflow,          // user defined (flow units),
    SMO_total_lateral_inflow,   // (sum of variables 4 to 8) //(flow units),
    SMO_flood_losses,           // (flow units),
    SMO_outfall_flows,          // (flow units),
    SMO_volume_stored,          // (ft3 or m3),
    SMO_evap_rate               // (in/day or mm/day),
	//p_evap_rate             // (in/day or mm/day)
} SMO_systemAttribute;


#endif /* SWMM_OUTPUT_ENUMS_H_ */
