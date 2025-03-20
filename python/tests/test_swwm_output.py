# Description: Unit tests for the SWMM output module
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# python imports
import json
import unittest
from datetime import datetime
import pickle

# third party imports

# local imports
from .data import output as example_output_data
from epaswmm import output
from epaswmm.output import Output, SWMMOutputException


class TestSWMMOutput(unittest.TestCase):
    """
    Test the SWMM solver functions

    TODO: Add tests to check for exceptions and errors
    """

    def setUp(self):
        """
        Set up the test case with the test artifacts
        :return:
        """

        self.test_artifacts = {}

        with open(file=example_output_data.JSON_TIME_SERIES_FILE, mode='rb') as f:
            self.test_artifacts = pickle.load(f)

    def test_output_unit_system_enum(self):
        """
        Test the output unit system enum
        :return:
        """

        self.assertEqual(
            first=output.UnitSystem.US.value, second=0, msg="US unit system value should be 0"
        )

        self.assertEqual(
            first=output.UnitSystem.SI.value, second=1, msg="SI unit system value should be 1"
        )

    def test_output_flow_units_enum(self):
        """
        Test the output flow units enum
        :return:
        """

        self.assertEqual(
            first=output.FlowUnits.CFS.value, second=0, msg="CFS flow unit value should be 0"
        )
        self.assertEqual(
            first=output.FlowUnits.GPM.value, second=1, msg="GPM flow unit value should be 1"
        )
        self.assertEqual(
            first=output.FlowUnits.MGD.value, second=2, msg="MGD flow unit value should be 2"
        )
        self.assertEqual(
            first=output.FlowUnits.CMS.value, second=3, msg="CMS flow unit value should be 3"
        )
        self.assertEqual(
            first=output.FlowUnits.LPS.value, second=4, msg="LPS flow unit value should be 4"
        )
        self.assertEqual(
            first=output.FlowUnits.MLD.value, second=5, msg="MLD flow unit value should be 5"
        )

    def test_output_concentration_units_enum(self):
        """
        Test the output concentration units enum
        :return:
        """

        self.assertEqual(
            first=output.ConcentrationUnits.MG.value, second=0, msg="MG concentration unit value should be 0"
        )
        self.assertEqual(
            first=output.ConcentrationUnits.UG.value, second=1, msg="UG concentration unit value should be 1"
        )
        self.assertEqual(
            first=output.ConcentrationUnits.COUNT.value, second=2, msg="COUNT concentration unit value should be 2"
        )
        self.assertEqual(
            first=output.ConcentrationUnits.NONE.value, second=3, msg="NONE concentration unit value should be 3"
        )

    def test_output_element_type_enum(self):
        """
        Test the output element type enum
        :return:
        """

        self.assertEqual(
            first=output.ElementType.SUBCATCHMENT.value, second=0, msg="SUBCATCHMENT element type value should be 0"
        )
        self.assertEqual(
            first=output.ElementType.NODE.value, second=1, msg="NODE element type value should be 1"
        )
        self.assertEqual(
            first=output.ElementType.LINK.value, second=2, msg="LINK element type value should be 2"
        )
        self.assertEqual(
            first=output.ElementType.SYSTEM.value, second=3, msg="SYSTEM element type value should be 3"
        )
        self.assertEqual(
            first=output.ElementType.POLLUTANT.value, second=4, msg="POLLUTANT element type value should be 4"
        )

    def test_output_time_enum(self):
        """
        Test the output time enum
        :return:
        """

        self.assertEqual(
            first=output.TimeAttribute.REPORT_STEP.value, second=0, msg="REPORT_STEP time value should be 0"
        )
        self.assertEqual(
            first=output.TimeAttribute.NUM_PERIODS.value, second=1, msg="NUM_PERIODS time value should be 1"
        )

    def test_output_sub_catch_attribute_enum(self):
        """
        Test the output sub-catchment attribute enum
        :return:
        """

        self.assertEqual(
            first=output.SubcatchAttribute.RAINFALL.value, second=0,
            msg="RAINFALL sub-catchment attribute value should be 0"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.SNOW_DEPTH.value, second=1,
            msg="SNOW_DEPTH sub-catchment attribute value should be 1"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.EVAPORATION_LOSS.value, second=2,
            msg="EVAPORATION_LOSS sub-catchment attribute value should be 2"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.INFILTRATION_LOSS.value, second=3,
            msg="INFILTRATION_LOSS sub-catchment attribute value should be 3"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.RUNOFF_RATE.value, second=4,
            msg="RUNOFF_RATE sub-catchment attribute value should be 4"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.GROUNDWATER_OUTFLOW.value, second=5,
            msg="GROUNDWATER_OUTFLOW sub-catchment attribute value should be 5"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.GROUNDWATER_TABLE_ELEVATION.value, second=6,
            msg="GROUNDWATER_TABLE sub-catchment attribute value should be 6"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.SOIL_MOISTURE.value, second=7,
            msg="SOIL_MOISTURE sub-catchment attribute value should be 7"
        )
        self.assertEqual(
            first=output.SubcatchAttribute.POLLUTANT_CONCENTRATION.value, second=8,
            msg="POLLUTANT_CONCENTRATION sub-catchment attribute value should be 8"
        )

    def test_output_node_attribute_enum(self):
        """
        Test the output node attribute enum
        :return:
        """

        self.assertEqual(
            first=output.NodeAttribute.INVERT_DEPTH.value, second=0,
            msg="INVERT_DEPTH node attribute value should be 0"
        )
        self.assertEqual(
            first=output.NodeAttribute.HYDRAULIC_HEAD.value, second=1,
            msg="HYDRAULIC_HEAD node attribute value should be 1"
        )
        self.assertEqual(
            first=output.NodeAttribute.STORED_VOLUME.value, second=2,
            msg="STORED_VOLUME node attribute value should be 2"
        )
        self.assertEqual(
            first=output.NodeAttribute.LATERAL_INFLOW.value, second=3,
            msg="LATERAL_INFLOW node attribute value should be 3"
        )
        self.assertEqual(
            first=output.NodeAttribute.TOTAL_INFLOW.value, second=4,
            msg="TOTAL_INFLOW node attribute value should be 4"
        )
        self.assertEqual(
            first=output.NodeAttribute.FLOODING_LOSSES.value, second=5,
            msg="FLOODING_LOSSES node attribute value should be 5"
        )
        self.assertEqual(
            first=output.NodeAttribute.POLLUTANT_CONCENTRATION.value, second=6,
            msg="POLLUTANT_CONCENTRATION node attribute value should be 6"
        )

    def test_output_link_attribute_enum(self):
        """
        Test the output link attribute enum
        :return:
        """

        self.assertEqual(
            first=output.LinkAttribute.FLOW_RATE.value, second=0,
            msg="FLOW_RATE link attribute value should be 0"
        )
        self.assertEqual(
            first=output.LinkAttribute.FLOW_DEPTH.value, second=1,
            msg="FLOW_DEPTH link attribute value should be 1"
        )
        self.assertEqual(
            first=output.LinkAttribute.FLOW_VELOCITY.value, second=2,
            msg="FLOW_VELOCITY link attribute value should be 2"
        )
        self.assertEqual(
            first=output.LinkAttribute.FLOW_VOLUME.value, second=3,
            msg="FLOW_VOLUME link attribute value should be 3"
        )
        self.assertEqual(
            first=output.LinkAttribute.CAPACITY.value, second=4,
            msg="CAPACITY link attribute value should be 4"
        )
        self.assertEqual(
            first=output.LinkAttribute.POLLUTANT_CONCENTRATION.value, second=5,
            msg="POLLUTANT_CONCENTRATION link attribute value should be 5"
        )

    def test_output_system_attribute_enum(self):
        """
        Test the output system attribute enum
        :return:
        """

        self.assertEqual(
            first=output.SystemAttribute.AIR_TEMP.value, second=0,
            msg="AIR_TEMP system attribute value should be 0"
        )
        self.assertEqual(
            first=output.SystemAttribute.RAINFALL.value, second=1,
            msg="RAINFALL system attribute value should be 1"
        )
        self.assertEqual(
            first=output.SystemAttribute.SNOW_DEPTH.value, second=2,
            msg="SNOW_DEPTH system attribute value should be 2"
        )
        self.assertEqual(
            first=output.SystemAttribute.EVAP_INFIL_LOSS.value, second=3,
            msg="EVAP_INFIL_LOSS system attribute value should be 3"
        )
        self.assertEqual(
            first=output.SystemAttribute.RUNOFF_FLOW.value, second=4,
            msg="RUNOFF_FLOW system attribute value should be 4"
        )
        self.assertEqual(
            first=output.SystemAttribute.DRY_WEATHER_INFLOW.value, second=5,
            msg="DRY_WEATHER_INFLOW system attribute value should be 5"
        )
        self.assertEqual(
            first=output.SystemAttribute.GROUNDWATER_INFLOW.value, second=6,
            msg="GROUNDWATER_INFLOW system attribute value should be 6"
        )
        self.assertEqual(
            first=output.SystemAttribute.RDII_INFLOW.value, second=7,
            msg="RDII_INFLOW system attribute value should be 7"
        )
        self.assertEqual(
            first=output.SystemAttribute.DIRECT_INFLOW.value, second=8,
            msg="DIRECT_INFLOW system attribute value should be 8"
        )
        self.assertEqual(
            first=output.SystemAttribute.TOTAL_LATERAL_INFLOW.value, second=9,
            msg="TOTAL_LATERAL_INFLOW system attribute value should be 9"
        )
        self.assertEqual(
            first=output.SystemAttribute.FLOOD_LOSSES.value, second=10,
            msg="FLOOD_LOSSES system attribute value should be 10"
        )
        self.assertEqual(
            first=output.SystemAttribute.OUTFALL_FLOWS.value, second=11,
            msg="OUTFALL_FLOWS system attribute value should be 11"
        )
        self.assertEqual(
            first=output.SystemAttribute.VOLUME_STORED.value, second=12,
            msg="VOLUME_STORED system attribute value should be 12"
        )
        self.assertEqual(
            first=output.SystemAttribute.EVAPORATION_RATE.value, second=13,
            msg="EVAPORATION_RATE system attribute value should be 13"
        )

    def test_output_open_and_close(self):
        """
        Test the output open and close functions
        :return:
        """
        with Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1) as swmm_output:
            pass

        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)

    def test_output_open_error(self):
        """
        Test the output open error function
        :return:
        """
        with self.assertRaises(expected_exception=FileNotFoundError) as context:
            swmm_output = Output(output_file=example_output_data.NON_EXISTENT_OUTPUT_FILE)

        self.assertIn(
            member="Error opening the SWMM output file",
            container=str(context.exception),
            msg="Error message should be 'Error opening the SWMM output file'"
        )

    def test_output_get_version(self):
        """
        Test the output get version function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        version = swmm_output.version

        self.assertEqual(first=version, second=51000, msg="Version should be 51000")

    def test_output_get_size(self):
        """
        Test the output get size function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        self.assertDictEqual(
            d1={'subcatchments': 8, 'nodes': 14, 'links': 13, 'system': 1, 'pollutants': 2},
            d2=swmm_output.output_size,
            msg="Output size should be {'subcatchments': 8, 'nodes': 14, 'links': 13, 'system': 1, 'pollutants': 2}"
        )

    def test_output_get_units(self):
        """
        Test the output get units function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        units = swmm_output.units

        self.assertListEqual(
            list1=list(units),
            list2=[
                output.UnitSystem.US,
                output.FlowUnits.CFS,
                [output.ConcentrationUnits.MG, output.ConcentrationUnits.UG]
            ],
            msg="Units should be [US, CFS, [MG, UG]]"
        )

    def test_output_get_flow_units(self):
        """
        Test the output get flow units function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        flow_units = swmm_output.flow_units

        self.assertEqual(
            first=flow_units, second=output.FlowUnits.CFS, msg="Flow units should be CFS"
        )

    def test_output_get_element_properties(self):
        """
        Test the output get element properties function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)

        sub_catchment_properties = swmm_output.get_num_properties(
            element_type=output.ElementType.SUBCATCHMENT
        )
        node_properties = swmm_output.get_num_properties(
            element_type=output.ElementType.NODE
        )
        link_properties = swmm_output.get_num_properties(
            element_type=output.ElementType.LINK
        )

        self.assertEqual(
            first=sub_catchment_properties, second=1,
            msg="Number of sub-catchment properties should be 1"
        )

        self.assertEqual(
            first=node_properties, second=3,
            msg="Number of node properties should be 3"
        )

        self.assertEqual(
            first=link_properties, second=5,
            msg="Number of link properties should be 5"
        )

        sub_catchment_property_codes = [
            swmm_output.get_property_code(
                element_type=output.ElementType.SUBCATCHMENT,
                property_index=i
            ) for i in range(sub_catchment_properties)
        ]

        sub_catchment_property_codes_all = swmm_output.get_property_codes(
            element_type=output.ElementType.SUBCATCHMENT
        )

        self.assertListEqual(
            list1=sub_catchment_property_codes,
            list2=[1],
            msg="Sub-catchment property codes should be [1]"
        )

        self.assertListEqual(
            list1=sub_catchment_property_codes,
            list2=sub_catchment_property_codes_all,
            msg="Sub-catchment property codes should be [1]"
        )

        node_property_codes = [
            swmm_output.get_property_code(
                element_type=output.ElementType.NODE,
                property_index=i
            ) for i in range(node_properties)
        ]

        node_property_codes_all = swmm_output.get_property_codes(
            element_type=output.ElementType.NODE
        )

        self.assertListEqual(
            list1=node_property_codes,
            list2=[0, 2, 3],
            msg="Node property codes should be [0, 2, 3]"
        )

        self.assertListEqual(
            list1=node_property_codes,
            list2=node_property_codes_all,
            msg="Node property codes should be [0, 2, 3]"
        )

        link_property_codes = [
            swmm_output.get_property_code(
                element_type=output.ElementType.LINK,
                property_index=i
            ) for i in range(link_properties)
        ]

        link_property_codes_all = swmm_output.get_property_codes(
            element_type=output.ElementType.LINK
        )

        self.assertListEqual(
            list1=link_property_codes,
            list2=[0, 4, 4, 3, 5],
            msg="Link property codes should be [0, 4, 4, 3, 5]"
        )

        self.assertListEqual(
            list1=link_property_codes,
            list2=link_property_codes_all,
            msg="Link property codes should be [0, 4, 4, 3, 5]"
        )

    def test_get_element_property_values(self):

        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)

        sub_catchment_property_values = [
            swmm_output.get_property_value(
                element_type=output.ElementType.SUBCATCHMENT,
                element_index=0,
                property_code=i
            ) for i in range(1)
        ]

        sub_catchment_property_values_all = swmm_output.get_property_values(
            element_type=output.ElementType.SUBCATCHMENT,
            element_index=0
        )

        self.assertListEqual(
            list1=sub_catchment_property_values,
            list2=[10.0],
            msg="Sub-catchment property values should be [10.0]"
        )

        self.assertListEqual(
            list1=sub_catchment_property_values,
            list2=sub_catchment_property_values_all,
            msg="Sub-catchment property values should be [10.0]"
        )

        node_property_values = [
            swmm_output.get_property_value(
                element_type=output.ElementType.NODE,
                element_index=0,
                property_code=i
            ) for i in range(3)
        ]

        node_property_values_all = swmm_output.get_property_values(
            element_type=output.ElementType.NODE,
            element_index=0
        )

        self.assertListEqual(
            list1=node_property_values,
            list2=[0.0, 1000.0, 3.0],
            msg="Node property values should be [0.0, 1000.0, 3.0]"
        )

        self.assertListEqual(
            list1=node_property_values,
            list2=node_property_values_all,
            msg="Node property values should be [0.0, 1000.0, 3.0]"
        )

        link_property_values = [
            swmm_output.get_property_value(
                element_type=output.ElementType.LINK,
                element_index=0,
                property_code=i
            ) for i in range(5)
        ]

        link_property_values_all = swmm_output.get_property_values(
            element_type=output.ElementType.LINK,
            element_index=0
        )

        self.assertListEqual(
            list1=link_property_values,
            list2=[0.0, 0.0, 0.0, 1.5, 400.0],
            msg="Link property values should be [0.0, 0.0, 0.0, 1.5, 400.0]"
        )

        self.assertListEqual(
            list1=link_property_values,
            list2=link_property_values_all,
            msg="Link property values should be [0.0, 0.0, 0.0, 1.5, 400.0]"
        )

    def test_output_get_element_variables(self):

        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        num_sub_catch_vars = swmm_output.get_num_variables(output.ElementType.SUBCATCHMENT)
        num_node_vars = swmm_output.get_num_variables(output.ElementType.NODE)
        num_link_vars = swmm_output.get_num_variables(output.ElementType.LINK)
        num_system_vars = swmm_output.get_num_variables(output.ElementType.SYSTEM)

        self.assertEqual(
            first=num_sub_catch_vars, second=10, msg="Number of sub-catchment variables should be 10"
        )

        self.assertEqual(
            first=num_node_vars, second=8, msg="Number of node variables should be 8"
        )

        self.assertEqual(
            first=num_link_vars, second=7, msg="Number of link variables should be 7"
        )

        self.assertEqual(
            first=num_system_vars, second=14, msg="Number of system variables should be 14"
        )

    def test_output_get_start_date(self):
        """
        Test the output get start date function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        start_date = swmm_output.start_date

        self.assertEqual(
            first=start_date,
            second=datetime(year=1998, month=1, day=1),
            msg="Start date should be 01/01/1998"
        )

    def test_output_get_time_attributes(self):
        """
        Test the output get temporal attributes function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        report_step = swmm_output.get_time_attribute(time_attribute=output.TimeAttribute.REPORT_STEP)
        num_periods = swmm_output.get_time_attribute(time_attribute=output.TimeAttribute.NUM_PERIODS)

        self.assertEqual(
            first=report_step, second=3600,
            msg="Report step should be 3600"
        )
        self.assertEqual(
            first=num_periods, second=36,
            msg="Number of periods should be 365"
        )

    def test_output_get_element_name(self):
        """
        Test the output get element names function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)

        retrieved_sub_catch_names = [
            swmm_output.get_element_name(
                element_type=output.ElementType.SUBCATCHMENT, element_index=i
            )
            for i in range(8)
        ]
        sub_catch_names = ['1', '2', '3', '4', '5', '6', '7', '8']

        self.assertListEqual(
            list1=retrieved_sub_catch_names, list2=sub_catch_names,
            msg="Sub-catchment names should be [1, 2, 3, 4, 5, 6, 7, 8]"
        )

        retrieved_node_names = [
            swmm_output.get_element_name(
                element_type=output.ElementType.NODE, element_index=i
            )
            for i in range(14)
        ]

        node_names = ['9', '10', '13', '14', '15', '16', '17', '19', '20', '21', '22', '23', '24', '18']

        self.assertListEqual(
            list1=retrieved_node_names, list2=node_names,
            msg="Node names should be [9, 10, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 18]"
        )

        retrieved_link_names = [
            swmm_output.get_element_name(
                element_type=output.ElementType.LINK, element_index=i
            ) for i in range(13)
        ]

        link_names = ['1', '4', '5', '6', '7', '8', '10', '11', '12', '13', '14', '15', '16']
        self.assertListEqual(
            list1=retrieved_link_names, list2=link_names,
            msg="Link names should be [1, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16]"
        )

        retrieved_pollutant_names = [
            swmm_output.get_element_name(
                element_type=output.ElementType.POLLUTANT, element_index=i
            )
            for i in range(2)
        ]

        pollutant_names = ['TSS', 'Lead']
        self.assertListEqual(
            list1=retrieved_pollutant_names, list2=pollutant_names,
            msg="Pollutant names should be [TSS, TSS]"
        )

    def test_get_element_name_errors(self):
        """
        Test the output get element name error function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)

        with self.assertRaises(expected_exception=Exception) as context:
            swmm_output.get_element_name(
                element_type=output.ElementType.SYSTEM, element_index=0
            )

        self.assertIn(
            member="invalid parameter code", container=str(context.exception),
            msg="Error message should be 'Invalid element type'"
        )

        with self.assertRaises(expected_exception=Exception) as context:
            swmm_output.get_element_name(
                element_type=output.ElementType.SUBCATCHMENT, element_index=8
            )

        self.assertIn(
            member="element index out of range",
            container=str(context.exception),
            msg="Error message should be 'Index out of range'"
        )

    def test_output_get_element_names(self):
        """
        Test the output get element names error function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)

        retrieved_sub_catch_names = swmm_output.get_element_names(element_type=output.ElementType.SUBCATCHMENT)
        sub_catch_names = ['1', '2', '3', '4', '5', '6', '7', '8']

        self.assertListEqual(
            list1=retrieved_sub_catch_names,
            list2=sub_catch_names,
            msg="Sub-catchment names should be [1, 2, 3, 4, 5, 6, 7, 8]"
        )

        retrieved_node_names = swmm_output.get_element_names(element_type=output.ElementType.NODE)
        node_names = [
            '9', '10', '13', '14', '15', '16', '17', '19', '20', '21', '22', '23', '24', '18'
        ]
        self.assertListEqual(
            list1=retrieved_node_names, list2=node_names,
            msg="Node names should be [9, 10, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 18]"
        )

        retrieved_link_names = swmm_output.get_element_names(element_type=output.ElementType.LINK)
        link_names = ['1', '4', '5', '6', '7', '8', '10', '11', '12', '13', '14', '15', '16']
        self.assertListEqual(
            list1=retrieved_link_names, list2=link_names,
            msg="Link names should be [1, 4, 5, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16]"
        )

        retrieved_pollutant_names = swmm_output.get_element_names(element_type=output.ElementType.POLLUTANT)
        pollutant_names = ['TSS', 'Lead']
        self.assertListEqual(
            list1=retrieved_pollutant_names, list2=pollutant_names,
            msg="Pollutant names should be [TSS, TSS]"
        )

        with self.assertRaises(expected_exception=SWMMOutputException) as context:
            swmm_output.get_element_names(element_type=output.ElementType.SYSTEM)

        self.assertIn(
            member="Cannot get element names for the system element type",
            container=str(context.exception),
            msg="Error message should be 'Invalid element type'"
        )

    def test_get_times(self):
        """
        Test the output get timeseries function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        times = swmm_output.times

        self.assertEqual(first=len(times), second=36, msg="Number of times should be 36")

        self.assertEqual(
            first=times[0], second=datetime(year=1998, month=1, day=1, hour=1),
            msg="First time should be 01/01/1998 01:00"
        )

        self.assertEqual(
            first=times[16], second=datetime(year=1998, month=1, day=1, hour=17),
            msg="Middle time should be 01/01/1998 14:00"
        )

        self.assertEqual(
            first=times[-1], second=datetime(year=1998, month=1, day=2, hour=12),
            msg="Last time should be 01/02/1998 12:00"
        )

    def test_get_sub_catchment_timeseries(self):
        """
        Test the output get sub-catchment timeseries function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        sub_catchment_timeseries = swmm_output.get_subcatchment_timeseries(
            element_index=5,
            attribute=output.SubcatchAttribute.RUNOFF_RATE,
        )

        sub_catchment_timeseries_by_name = swmm_output.get_subcatchment_timeseries(
            element_index='6',
            attribute=output.SubcatchAttribute.RUNOFF_RATE,
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=sub_catchment_timeseries,
            d2=self.test_artifacts['test_get_subcatchment_timeseries'],
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=sub_catchment_timeseries_by_name,
            d2=sub_catchment_timeseries,
        )

    def test_get_node_timeseries(self):
        """
        Test the output get node timeseries function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        node_timeseries = swmm_output.get_node_timeseries(
            element_index=7,
            attribute=output.NodeAttribute.TOTAL_INFLOW,
        )

        node_timeseries_by_name = swmm_output.get_node_timeseries(
            element_index='19',
            attribute=output.NodeAttribute.TOTAL_INFLOW,
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=node_timeseries,
            d2=self.test_artifacts['test_get_node_timeseries'],
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=node_timeseries_by_name,
            d2=node_timeseries,
        )

    def test_get_link_timeseries(self):
        """
        Test the output get link timeseries function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        link_timeseries = swmm_output.get_link_timeseries(
            element_index=5,
            attribute=output.LinkAttribute.FLOW_RATE,
        )

        link_timeseries_by_name = swmm_output.get_link_timeseries(
            element_index='8',
            attribute=output.LinkAttribute.FLOW_RATE,
        )

        TestSWMMOutput.assert_dict_almost_equal(
            link_timeseries,
            self.test_artifacts['test_get_link_timeseries'],
        )

        TestSWMMOutput.assert_dict_almost_equal(
            link_timeseries_by_name,
            link_timeseries,
        )

    def test_get_system_timeseries(self):
        """
        Test the output get system timeseries function
        :return:
        """
        swmm_output = Output(example_output_data.EXAMPLE_OUTPUT_FILE_1)
        system_timeseries = swmm_output.get_system_timeseries(
            attribute=output.SystemAttribute.RUNOFF_FLOW
        )

        TestSWMMOutput.assert_dict_almost_equal(
            system_timeseries,
            self.test_artifacts['test_get_system_timeseries'],
        )

    def test_get_sub_catchment_values_by_time_and_attributes(self):
        """
        Test the output get sub-catchment values by time and attributes function
        :return:
        """
        swmm_output = Output(example_output_data.EXAMPLE_OUTPUT_FILE_1)
        sub_catchment_values = swmm_output.get_subcatchment_values_by_time_and_attribute(
            time_index=5,
            attribute=output.SubcatchAttribute.RUNOFF_RATE
        )

        TestSWMMOutput.assert_dict_almost_equal(
            sub_catchment_values,
            self.test_artifacts['test_get_subcatchment_values_by_time_and_attributes'],
        )

    def test_get_node_values_by_time_and_attributes(self):
        """
        Test the output get node values by time and attributes function
        :return:
        """
        swmm_output = Output(example_output_data.EXAMPLE_OUTPUT_FILE_1)
        node_values = swmm_output.get_node_values_by_time_and_attribute(
            time_index=8,
            attribute=output.NodeAttribute.TOTAL_INFLOW
        )

        TestSWMMOutput.assert_dict_almost_equal(
            node_values,
            self.test_artifacts['test_get_node_values_by_time_and_attributes'],
        )

    def test_get_link_values_by_time_and_attributes(self):
        """
        Test the output get link values by time and attributes function
        :return:
        """
        swmm_output = Output(example_output_data.EXAMPLE_OUTPUT_FILE_1)
        link_values = swmm_output.get_link_values_by_time_and_attribute(
            time_index=10,
            attribute=output.LinkAttribute.FLOW_RATE
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=link_values,
            d2=self.test_artifacts['test_get_link_values_by_time_and_attributes'],
        )

    def test_get_system_values_by_time_and_attributes(self):
        """
        Test the output get system values by time and attributes function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        system_values = swmm_output.get_system_values_by_time_and_attribute(
            time_index=12,
            attribute=output.SystemAttribute.RUNOFF_FLOW
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=system_values,
            d2=self.test_artifacts['test_get_system_values_by_time_and_attributes'],
        )

    def test_get_sub_catchment_values_by_time_and_index(self):
        """
        Test the output get sub-catchment values by time and index function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        sub_catchment_values = swmm_output.get_subcatchment_values_by_time_and_element_index(
            time_index=5,
            element_index=3
        )

        sub_catchment_values_by_name = swmm_output.get_subcatchment_values_by_time_and_element_index(
            time_index=5,
            element_index='4'
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=sub_catchment_values,
            d2=self.test_artifacts['test_get_subcatchment_values_by_time_and_index'],
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=sub_catchment_values_by_name,
            d2=sub_catchment_values,
        )

    def test_get_node_values_by_time_and_index(self):
        """
        Test the output get node values by time and index function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        node_values = swmm_output.get_node_values_by_time_and_element_index(
            time_index=8,
            element_index=4
        )

        node_values_by_name = swmm_output.get_node_values_by_time_and_element_index(
            time_index=8,
            element_index='15'
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=node_values,
            d2=self.test_artifacts['test_get_node_values_by_time_and_index'],
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=node_values_by_name,
            d2=node_values,
        )

    def test_get_link_values_by_time_and_index(self):
        """
        Test the output get link values by time and index function
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        link_values = swmm_output.get_link_values_by_time_and_element_index(
            time_index=10,
            element_index=5
        )

        link_values_by_name = swmm_output.get_link_values_by_time_and_element_index(
            time_index=10,
            element_index='8'
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=link_values,
            d2=self.test_artifacts['test_get_link_values_by_time_and_index'],
        )

        TestSWMMOutput.assert_dict_almost_equal(
            d1=link_values_by_name,
            d2=link_values,
        )

    def test_get_system_values_by_time(self):
        """
        Test the output get system values by time
        :return:
        """
        swmm_output = Output(output_file=example_output_data.EXAMPLE_OUTPUT_FILE_1)
        system_values = swmm_output.get_system_values_by_time(time_index=12)

        TestSWMMOutput.assert_dict_almost_equal(
            d1=system_values,
            d2=self.test_artifacts['test_get_system_values_by_time']
        )

    @staticmethod
    def assert_dict_almost_equal(d1: dict, d2: dict, rtol: float = 1e-5, atol: float = 1e-8):
        """
        Assert that two dictionaries are almost equal
        :param d1: First dictionary to compare with d2 dictionary
        :param d2: Second dictionary to compare with d1 dictionary
        :param rtol: Relative error tolerance for floating point values
        :param atol: Absolute error tolerance for floating point values
        :return:
        """
        """Assert that two dictionaries are almost equal (with tolerance)."""

        assert set(d1.keys()) == set(d2.keys())  # Check if keys are the same

        for key in d1.keys():
            value1 = d1[key]
            value2 = d2[key]

            if isinstance(value1, dict):
                # If the values are dictionaries, recursively compare them
                TestSWMMOutput.assert_dict_almost_equal(value1, value2, rtol, atol)
            elif isinstance(value1, float):
                # If the values are floats, compare them with tolerance
                assert abs(value1 - value2) <= atol + rtol * abs(value2)
            else:
                # Otherwise, compare them directly
                assert value1 == value2
