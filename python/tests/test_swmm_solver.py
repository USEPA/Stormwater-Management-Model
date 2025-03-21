# Description: Unit tests for the epaswmm solver module.
# Created by: Caleb Buahin (EPA/ORD/CESER/WID)
# Created on: 2024-11-19

# python imports
import unittest
from datetime import datetime

# third party imports
import os
import sys

# local imports
from .data import solver as example_solver_data
from epaswmm import solver


class TestSWMMSolver(unittest.TestCase):

    def setUp(self):
        self.swmm_test_datetime = datetime(year=2024, month=11, day=16, hour=13, minute=33, second=21)
        self.swmm_test_double_datetime = 45612.564826389

        self.site_drainage_inp = example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE
        self.site_drainage_rpt = example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".rpt")
        self.site_drainage_out = example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out")

    @staticmethod
    def progress_callback(progress: float) -> None:
        """
        Progress callback function for the SWMM solver
        :param progress:
        :return:
        """
        assert 0 <= progress <= 1.0

    def test_get_swmm_version(self):
        """
        Test the version function of the SWMM solver
        :return:
        """
        version = solver.version()
        self.assertEqual(version, 53000, "SWMM version retrieved successfully")

    def test_swmm_encode_date(self):
        """
        Test the encode_swmm_datetime function
        :return:
        """

        swmm_datetime_encoded = solver.encode_swmm_datetime(dt=self.swmm_test_datetime)
        self.assertAlmostEqual(swmm_datetime_encoded, self.swmm_test_double_datetime)

    def test_swmm_decode_date(self):
        """
        Test the decode_swmm_datetime function
        :return:
        """
        swmm_datetime = solver.decode_swmm_datetime(
            swmm_datetime=self.swmm_test_double_datetime
        )

        self.assertEqual(swmm_datetime, self.swmm_test_datetime)

    def test_run_solver(self):
        """
        Run the SWMM solver to solve the example input file

        :return:
        """

        if os.path.exists(self.site_drainage_rpt):
            os.remove(self.site_drainage_rpt)

        if os.path.exists(self.site_drainage_out):
            os.remove(self.site_drainage_out)

        error = solver.run_solver(
            inp_file=self.site_drainage_inp,
            rpt_file=self.site_drainage_rpt,
            out_file=self.site_drainage_out,
        )

        self.assertEqual(error, 0, "SWMM solver run successfully.")

        # Assert output and report files were created
        self.assertTrue(os.path.exists(self.site_drainage_rpt))
        self.assertTrue(os.path.exists(self.site_drainage_out))

    def test_run_solver_with_progress_callback(self):
        """
        Run the SWMM solver to solve the example input file with progress callback

        :return:
        """

        if os.path.exists(self.site_drainage_rpt):
            os.remove(self.site_drainage_rpt)

        if os.path.exists(self.site_drainage_out):
            os.remove(self.site_drainage_out)

        error = solver.run_solver(
            inp_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE,
            rpt_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".rpt"),
            out_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out"),
            swmm_progress_callback=self.progress_callback
        )

        self.assertEqual(error, 0, "SWMM solver with callbacks run successfully.")

        # Assert output and report files were created
        self.assertTrue(os.path.exists(self.site_drainage_rpt))
        self.assertTrue(os.path.exists(self.site_drainage_out))

    def test_run_solver_invalid_inp_file(self):
        """
        Run the SWMM solver with an invalid input file path to test error handling
        :return:
        """

        with self.assertRaises(Exception) as context:
            report_file = example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".rpt")
            output_file = example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".out")

            solver.run_solver(
                inp_file=example_solver_data.NON_EXISTENT_INPUT_FILE,
                rpt_file=report_file,
                out_file=output_file,
            )

        self.assertIn('ERROR 303: cannot open input file.', str(context.exception))

    def test_run_without_context_manager(self):
        """
        Run the SWMM solver without a context manager
        :return:
        """

        if os.path.exists(self.site_drainage_rpt):
            os.remove(self.site_drainage_rpt)

        if os.path.exists(self.site_drainage_out):
            os.remove(self.site_drainage_out)

        swmm_solver = solver.Solver(
            inp_file=self.site_drainage_inp,
            rpt_file=self.site_drainage_rpt,
            out_file=self.site_drainage_out,
        )

        swmm_solver.execute()

        # Assert output and report files were created
        self.assertTrue(os.path.exists(self.site_drainage_rpt))
        self.assertTrue(os.path.exists(self.site_drainage_out))

    def test_run_without_context_manager_step_by_step(self):
        """
        Run the SWMM solver without a context manager and an invalid input file path to test error handling
        :return:
        """
        if os.path.exists(self.site_drainage_rpt):
            os.remove(self.site_drainage_rpt)

        if os.path.exists(self.site_drainage_out):
            os.remove(self.site_drainage_out)

        swmm_solver = solver.Solver(
            inp_file=self.site_drainage_inp,
            rpt_file=self.site_drainage_rpt,
            out_file=self.site_drainage_out,
        )

        swmm_solver.initialize()

        while swmm_solver.solver_state != solver.SolverState.FINISHED:
            swmm_solver.step()

        swmm_solver.finalize()

        # Assert output and report files were created
        self.assertTrue(os.path.exists(self.site_drainage_rpt))
        self.assertTrue(os.path.exists(self.site_drainage_out))

    def test_run_solver_with_context_manager(self):
        """
        Run the SWMM solver with an invalid report file path to test error handling
        :return:
        """

        with solver.Solver(
                inp_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE,
                rpt_file=example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".rpt"),
                out_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out"),
        ) as swmm_solver:

            swmm_solver.start()

            for _ in swmm_solver:
                pass

    def test_solver_get_time_attributes(self):
        """
        Test the get_start_date function of the SWMM solver
        :return:
        """
        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out
        ) as swmm_solver:
            # Initialize the solver
            swmm_solver.start()

            start_date = swmm_solver.start_datetime
            end_date = swmm_solver.end_datetime
            report_start_date = swmm_solver.report_start_datetime

            self.assertEqual(start_date, datetime(year=1998, month=1, day=1))
            self.assertEqual(end_date, datetime(year=1998, month=1, day=1, hour=6))
            self.assertEqual(report_start_date, datetime(year=1998, month=1, day=1))

    def test_solver_set_time_attributes(self):
        """
        Test the set_start_date function of the SWMM solver
        :return:
        """
        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out
        ) as swmm_solver:
            # Initialize the solver

            start_date = datetime(year=2000, month=1, day=2)
            end_date = datetime(year=2000, month=1, day=2, hour=2)
            report_start_date = datetime(year=2000, month=1, day=2)

            swmm_solver.start_datetime = start_date
            swmm_solver.end_datetime = end_date
            swmm_solver.report_start_datetime = report_start_date

            swmm_solver.start()

            for i, t in enumerate(swmm_solver):
                current_datetime = swmm_solver.current_datetime
                self.assertTrue(start_date <= current_datetime <= end_date, "Current datetime is within the range")

    def test_get_object_count(self):
        """
        Test the get_object_count function of the SWMM solver
        :return:
        """
        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out
        ) as swmm_solver:

            swmm_solver.start()

            num_raingages = swmm_solver.get_object_count(solver.SWMMObjects.RAIN_GAGE)
            num_subcatchments = swmm_solver.get_object_count(solver.SWMMObjects.SUBCATCHMENT)
            num_nodes = swmm_solver.get_object_count(solver.SWMMObjects.NODE)
            num_links = swmm_solver.get_object_count(solver.SWMMObjects.LINK)
            num_aquifers = swmm_solver.get_object_count(solver.SWMMObjects.AQUIFER)
            num_snowpacks = swmm_solver.get_object_count(solver.SWMMObjects.SNOWPACK)
            num_hydrographs = swmm_solver.get_object_count(solver.SWMMObjects.UNIT_HYDROGRAPH)
            num_lids = swmm_solver.get_object_count(solver.SWMMObjects.LID)
            num_streets = swmm_solver.get_object_count(solver.SWMMObjects.STREET)
            num_inlets = swmm_solver.get_object_count(solver.SWMMObjects.INLET)
            num_transects = swmm_solver.get_object_count(solver.SWMMObjects.TRANSECT)
            num_xsections = swmm_solver.get_object_count(solver.SWMMObjects.XSECTION_SHAPE)
            num_controls = swmm_solver.get_object_count(solver.SWMMObjects.CONTROL_RULE)
            num_pollutants = swmm_solver.get_object_count(solver.SWMMObjects.POLLUTANT)
            num_landuses = swmm_solver.get_object_count(solver.SWMMObjects.LANDUSE)
            num_curves = swmm_solver.get_object_count(solver.SWMMObjects.CURVE)
            num_timeseries = swmm_solver.get_object_count(solver.SWMMObjects.TIMESERIES)
            num_time_patterns = swmm_solver.get_object_count(solver.SWMMObjects.TIME_PATTERN)

            self.assertEqual(num_raingages, 1)
            self.assertEqual(num_subcatchments, 7)
            self.assertEqual(num_nodes, 12)
            self.assertEqual(num_links, 11)
            self.assertEqual(num_aquifers, 0)
            self.assertEqual(num_snowpacks, 0)
            self.assertEqual(num_hydrographs, 0)
            self.assertEqual(num_lids, 0)
            self.assertEqual(num_streets, 0)
            self.assertEqual(num_inlets, 0)
            self.assertEqual(num_transects, 0)
            self.assertEqual(num_xsections, 0)
            self.assertEqual(num_controls, 0)
            self.assertEqual(num_pollutants, 1)
            self.assertEqual(num_landuses, 4)
            self.assertEqual(num_curves, 0)
            self.assertEqual(num_timeseries, 3)
            self.assertEqual(num_time_patterns, 0)

            with self.assertRaises(solver.SWMMSolverException) as context:
                swmm_solver.get_object_count(solver.SWMMObjects.SYSTEM)

            self.assertIn('API Error -999904: invalid object type.', str(context.exception))

    def test_get_object_names(self):
        """
        Test the get_object_names function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:
            swmm_solver.start()

            raingage_names = swmm_solver.get_object_names(solver.SWMMObjects.RAIN_GAGE)
            subcatchment_names = swmm_solver.get_object_names(solver.SWMMObjects.SUBCATCHMENT)
            node_names = swmm_solver.get_object_names(solver.SWMMObjects.NODE)
            link_names = swmm_solver.get_object_names(solver.SWMMObjects.LINK)
            pollutant_names = swmm_solver.get_object_names(solver.SWMMObjects.POLLUTANT)
            landuse_names = swmm_solver.get_object_names(solver.SWMMObjects.LANDUSE)
            timeseries_names = swmm_solver.get_object_names(solver.SWMMObjects.TIMESERIES)

            self.assertListEqual(raingage_names, ['RainGage'])
            self.assertListEqual(subcatchment_names, ['S1', 'S2', 'S3', 'S4', 'S5', 'S6', 'S7'])
            self.assertListEqual(node_names, ['J1', 'J2', 'J3', 'J4', 'J5', 'J6', 'J7', 'J8', 'J9', 'J10', 'J11', 'O1'])
            self.assertListEqual(link_names, ['C1', 'C2', 'C3', 'C4', 'C5', 'C6', 'C7', 'C8', 'C9', 'C10', 'C11'])
            self.assertListEqual(pollutant_names, ['TSS'])
            self.assertListEqual(landuse_names, ['Residential_1', 'Residential_2', 'Commercial', 'Undeveloped'])
            self.assertListEqual(timeseries_names, ['2-yr', '10-yr', '100-yr'])

    def test_get_object_index(self):
        """
        Test the get_object_index function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:
            swmm_solver.start()

            rg_index = swmm_solver.get_object_index(solver.SWMMObjects.RAIN_GAGE, 'RainGage')
            sc_index = swmm_solver.get_object_index(solver.SWMMObjects.SUBCATCHMENT, 'S2')
            node_index = swmm_solver.get_object_index(solver.SWMMObjects.NODE, 'J6')
            link_index = swmm_solver.get_object_index(solver.SWMMObjects.LINK, 'C10')
            pollutant_index = swmm_solver.get_object_index(solver.SWMMObjects.POLLUTANT, 'TSS')
            landuse_index = swmm_solver.get_object_index(solver.SWMMObjects.LANDUSE, 'Commercial')
            timeseries_index = swmm_solver.get_object_index(solver.SWMMObjects.TIMESERIES, '10-yr')

            self.assertEqual(rg_index, 0)
            self.assertEqual(sc_index, 1)
            self.assertEqual(node_index, 5)
            self.assertEqual(link_index, 9)
            self.assertEqual(pollutant_index, 0)
            self.assertEqual(landuse_index, 2)
            self.assertEqual(timeseries_index, 1)

    def test_get_gage_value(self):
        """
        Test the get_gage_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:
            swmm_solver.start()

            for t in range(12):
                swmm_solver.step()

            rg_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.RAIN_GAGE,
                property_type=solver.SWMMRainGageProperties.GAGE_TOTAL_PRECIPITATION,
                index=0,
            )

            self.assertAlmostEqual(rg_value / 12.0, 0.3)

    def test_set_gage_value(self):
        """
        Test the set_gage_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:
            swmm_solver.start()

            swmm_solver.set_value(
                object_type=solver.SWMMObjects.RAIN_GAGE,
                property_type=solver.SWMMRainGageProperties.GAGE_RAINFALL,
                index=0,
                value=3.6
            )

            for _ in range(12):
                swmm_solver.step()

            rg_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.RAIN_GAGE,
                property_type=solver.SWMMRainGageProperties.GAGE_TOTAL_PRECIPITATION,
                index=0,
            )

            self.assertAlmostEqual(rg_value, 3.6)

    def test_get_sub_catchment_value(self):
        """
        Test the get_subcatchment_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:
            swmm_solver.start()

            for t in range(12):
                swmm_solver.step()

            sc_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.SUBCATCHMENT,
                property_type=solver.SWMMSubcatchmentProperties.RUNOFF,
                index=1,
            )

            self.assertAlmostEqual(sc_value, 17.527141504933294)

    def test_set_sub_catchment_value(self):
        """
        Test the set_subcatchment_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:

            swmm_solver.set_value(
                object_type=solver.SWMMObjects.SUBCATCHMENT,
                property_type=solver.SWMMSubcatchmentProperties.WIDTH,
                index=1,
                value=100.0
            )

            swmm_solver.start()

            for _ in range(12):
                swmm_solver.step()

            sc_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.SUBCATCHMENT,
                property_type=solver.SWMMSubcatchmentProperties.WIDTH,
                index=1,
            )

            self.assertAlmostEqual(sc_value, 100.0)

    def test_get_sub_catchment_initial_buildup(self):
        """
        Test the get_subcatchment_initial_buildup function of the SWMM solver

        :return:
        """

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:
            swmm_solver.start()

            sc_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.SUBCATCHMENT,
                property_type=solver.SWMMSubcatchmentProperties.POLLUTANT_BUILDUP,
                index=0,
                sub_index=0,
            )

            self.assertEqual(first=sc_value, second=80.0)

    def test_set_sub_catchment_initial_buildup(self):
        """
        Test the get_subcatchment_initial_buildup function of the SWMM solver

        :return:
        """

        # import matplotlib
        # matplotlib.use('TkAgg')
        #
        # import matplotlib.pyplot as plt
        # import pandas as pd
        #
        # fig, axes = plt.subplots(nrows=2, ncols=1 , sharex=True)

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:

            swmm_solver.stride_step = 10000

            swmm_solver.start()

            for _ in swmm_solver:
                pass

        from epaswmm import output
        toutput = output.Output(output_file=self.site_drainage_out)

        runoff = toutput.get_subcatchment_timeseries(
            element_index='S1',
            attribute=output.SubcatchAttribute.INFILTRATION_LOSS,
            sub_index=0
        )

        results = toutput.get_subcatchment_timeseries(
            element_index='S1',
            attribute=output.SubcatchAttribute.POLLUTANT_CONCENTRATION,
            sub_index=1
        )
        del toutput

        # runoff = pd.Series(runoff, index=pd.to_datetime(list(runoff.keys())))
        # runoff = pd.DataFrame(runoff, columns=['Runoff (cfs)'])
        #
        # runoff.plot()
        # plt.show()
        # matplotlib.use('TkAgg')
        #
        # runoff.plot(ax=axes[0])

        # data = pd.Series(results, index=pd.to_datetime(list(results.keys())))
        # data = pd.DataFrame(data, columns=['TSS Original (mg/L)'])
        # data.plot(ax=axes[1], linewidth=3.0)

        with solver.Solver(
                inp_file=self.site_drainage_inp,
                rpt_file=self.site_drainage_rpt,
                out_file=self.site_drainage_out,
        ) as swmm_solver:

            swmm_solver.start()

            for t, elapsed_time in enumerate(swmm_solver):

                if t == 10:
                    swmm_solver.set_value(
                        object_type=solver.SWMMObjects.SUBCATCHMENT,
                        property_type=solver.SWMMSubcatchmentProperties.EXTERNAL_POLLUTANT_BUILDUP,
                        index='S1',
                        value=120.0,
                        sub_index=0,
                    )
                    swmm_solver.step(1)
                    swmm_solver.set_value(
                        object_type=solver.SWMMObjects.SUBCATCHMENT,
                        property_type=solver.SWMMSubcatchmentProperties.EXTERNAL_POLLUTANT_BUILDUP,
                        index='S1',
                        value=0.0,
                        sub_index=0,
                    )
                else:
                    pass

        toutput = output.Output(output_file=self.site_drainage_out)
        results = toutput.get_subcatchment_timeseries(
            element_index='S1',
            attribute=output.SubcatchAttribute.POLLUTANT_CONCENTRATION,
            sub_index=1
        )
        del toutput

        # rdata = pd.Series(results, index=pd.to_datetime(list(results.keys())))
        # rdata = pd.DataFrame(rdata, columns=['TSS Modified (mg/L)'])
        # rdata.plot(ax=axes[1])
        #
        # plt.show()
        # matplotlib.use('TkAgg')

    def test_get_node_value(self):
        """
        Test the get_node_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE,
                rpt_file=example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".rpt"),
                out_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out"),
        ) as swmm_solver:
            swmm_solver.start()

            for t in range(12):
                swmm_solver.step()

            node_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.NODE,
                property_type=solver.SWMMNodeProperties.TOTAL_INFLOW,
                index=5,
            )

            self.assertAlmostEqual(node_value, 58.58717843671191)

    def test_set_node_value(self):
        """
        Test the set_node_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE,
                rpt_file=example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".rpt"),
                out_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out"),
        ) as swmm_solver:

            swmm_solver.set_value(
                object_type=solver.SWMMObjects.NODE,
                property_type=solver.SWMMNodeProperties.INVERT_ELEVATION,
                index=5,
                value=10.0
            )

            swmm_solver.start()

            for _ in range(12):
                swmm_solver.step()

            node_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.NODE,
                property_type=solver.SWMMNodeProperties.INVERT_ELEVATION,
                index=5,
            )

            self.assertAlmostEqual(node_value, 10.0)

    def test_get_link_value(self):
        """
        Test the get_link_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE,
                rpt_file=example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".rpt"),
                out_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out"),
        ) as swmm_solver:
            swmm_solver.start()

            for t in range(12):
                swmm_solver.step()

            link_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.LINK,
                property_type=solver.SWMMLinkProperties.FLOW,
                index=9,
            )

            link_value_by_name = swmm_solver.get_value(
                object_type=solver.SWMMObjects.LINK,
                property_type=solver.SWMMLinkProperties.FLOW,
                index='C10',
            )

            self.assertAlmostEqual(first=link_value, second=102.01283173880869)
            self.assertAlmostEqual(first=link_value, second=link_value_by_name)

    def test_set_link_value(self):
        """
        Test the set_link_value function of the SWMM solver
        :return:
        """

        with solver.Solver(
                inp_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE,
                rpt_file=example_solver_data.NON_EXISTENT_INPUT_FILE.replace(".inp", ".rpt"),
                out_file=example_solver_data.SITE_DRAINAGE_EXAMPLE_INPUT_FILE.replace(".inp", ".out"),
        ) as swmm_solver:

            swmm_solver.set_value(
                object_type=solver.SWMMObjects.LINK,
                property_type=solver.SWMMLinkProperties.START_NODE_OFFSET,
                index=9,
                value=1.0
            )

            swmm_solver.start()

            for _ in range(12):
                swmm_solver.step()

            link_value = swmm_solver.get_value(
                object_type=solver.SWMMObjects.LINK,
                property_type=solver.SWMMLinkProperties.START_NODE_OFFSET,
                index=9,
            )

            self.assertAlmostEqual(link_value, 1.0)
