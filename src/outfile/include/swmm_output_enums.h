/*!
 *  \file swmm_output_enums.h
 *  \author Michael Tryby (US EPA - ORD/CESER)
 *  \author Caleb Buahin (US EPA - ORD/CESER) (Editor)
 *  \date Created On: 10/18/2019
 *  \date Last Edited: 05/31/2023
 *  \brief Header file for SWMM output enumeration types
 */

#ifndef SWMM_OUTPUT_ENUMS_H_
#define SWMM_OUTPUT_ENUMS_H_

/*!
 * \brief Unit system used in the simulation
 */
typedef enum
{
    /*! \brief US Customary units (ft, acre, etc.)*/
    SMO_US,
    /*! \brief International System of Units (m, ha, etc.)*/
    SMO_SI
} SMO_unitSystem;

/*!
 * \brief Flow units used in the simulation
 */
typedef enum
{
    /*! \brief Cubic feet per second*/
    SMO_CFS,
    /*! \brief Gallons per minute*/
    SMO_GPM,
    /*! \brief Million gallons per minute*/
    SMO_MGD,
    /*! \brief Cubic meters per second*/
    SMO_CMS,
    /*! \brief Liters per second*/
    SMO_LPS,
    /*! \brief Million liters per day*/
    SMO_MLD
} SMO_flowUnits;

/*!
 * \brief Concentration units used in the simulation
 */
typedef enum
{
    /*! \brief Milligrams per liter*/
    SMO_MG,
    /*! \brief Micrograms per liter*/
    SMO_UG,
    /*! \brief Count per liter*/
    SMO_COUNT,
    /*! \brief No units*/
    SMO_NONE
} SMO_concUnits;

/*!
 * \brief SWMM Element types
 */
typedef enum
{
    /*! \brief SWMM Subcatchment*/
    SMO_subcatch,
    /*! \brief SWMM Node*/
    SMO_node,
    /*! \brief SWMM Link*/
    SMO_link,
    /*! \brief SWMM System*/
    SMO_sys,
    /*! \brief SWMM Pollutants*/
    SMO_pollut
} SMO_elementType;

/*!
 * \brief SWMM report time attributes
 */
typedef enum
{
    /*! \brief Report step size (seconds)*/
    SMO_reportStep,
    /*! \brief Number of reporting periods*/
    SMO_numPeriods
} SMO_time;

/*!
 * \brief SWMM Subcatchment attributes
 */
typedef enum
{
    /*! \brief Subcatchment rainfall in (in / hr or mm / hr) */
    SMO_rainfall_subcatch,
    /*! \brief Subcatchment snow depth in (in or mm) */
    SMO_snow_depth_subcatch,
    /*! \brief Subcatchment evaporation loss (in / hr or mm / hr)*/
    SMO_evap_loss,
    /*! \brief Subcatchment infiltration loss (in / hr or mm / hr) */
    SMO_infil_loss,
    /*! \brief Subcatchment runoff (flow units) */
    SMO_runoff_rate,
    /*! \brief Subcatchment groundwater outflow (flow units) */
    SMO_gwoutflow_rate,
    /*! \brief Subcatchment groundwater table elevation (ft or m) */
    SMO_gwtable_elev,
    /*! \brief Subcatchment soil moisture content (-) */
    SMO_soil_moisture,
    /*! \brief Subcatchment pollutant concentration */
    SMO_pollutant_conc_subcatch
} SMO_subcatchAttribute;

/*!
 * \brief SWMM Node attributes
 */
typedef enum
{
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
typedef enum
{
    /*!
     * \brief Link flow rate in (flow units)
     */
    SMO_flow_rate_link, // (flow units),
    /*!
     * \brief Link flow depth in (ft or m)
     * \note Flow depth is the depth of flow in the link's conduit
     * \sa SMO_flow_volume
     * \sa SMO_flow_velocity
     */
    SMO_flow_depth, // (ft or m),

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
    SMO_pollutant_conc_link // first pollutant,
} SMO_linkAttribute;

/*!
 * \brief SWMM System attributes
 */
typedef enum
{
    /*!
     * \brief System air temperature in (deg. F or deg. C)
     */
    SMO_air_temp,
    /*!
     * \brief System rainfall in (in/hr or mm/hr)
     * \note Rainfall is the subcatchment area weighted total rainfall over the entire study area
     */
    SMO_rainfall_system,

    /*!
     * \brief System snow depth in (in or mm)
     * \note Snow depth is the subcatchment area weighted total snow depth over the entire study area
     * \sa SMO_rainfall_system
     */
    SMO_snow_depth_system,

    /*!
     * \brief System evaporation loss in (in/hr or mm/hr)
     */
    SMO_evap_infil_loss,

    /*!
     * \brief System runoff flow in (flow units)
     */
    SMO_runoff_flow,

    /*!
     * \brief System dry weather inflow in (flow units)
     */
    SMO_dry_weather_inflow,

    /*!
     * \brief System groundwater inflow in (flow units)
     */
    SMO_groundwater_inflow,

    /*!
     * \brief System RDII inflow in (flow units)
     */
    SMO_RDII_inflow,

    /*!
     * \brief System direct inflow in (flow units)
     */
    SMO_direct_inflow,

    /*!
     * \brief System total lateral inflow in (flow units)
     */
    SMO_total_lateral_inflow,

    /*!
     * \brief System flooding losses in (flow units)
     */
    SMO_flood_losses,

    /*!
     * \brief System outfall flow in (flow units)
     */
    SMO_outfall_flows,

    /*!
     * \brief System volume stored in (ft3 or m3)
     */
    SMO_volume_stored,

    /*!
     * \brief System evaporation rate in (in/day or mm/day)
     */
    SMO_evap_rate,

    /*!
     * \brief System evaporation rate in (in/day or mm/day)
     * \deprecated Since? Use SMO_evap_rate instead
     */
    // p_evap_rate // (in/day or mm/day)
} SMO_systemAttribute;

#endif /* SWMM_OUTPUT_ENUMS_H_ */
