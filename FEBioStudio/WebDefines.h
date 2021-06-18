/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#ifdef WIN32
    #define MANUAL_PATH "/../doc/htmlManual/"
#elif __APPLE__
    #define MANUAL_PATH "/../../../doc/htmlManual/"
#else
    #define MANUAL_PATH "/../doc/htmlManual/"
#endif

#define INTRODUCTION_HTML "Introduction.html"
#define OVERVIEW_OF_FEBIO_HTML "Overview_of_FEBio.html"
#define ABOUT_THIS_DOCUMENT_HTML "About_this_document.html"
#define FEBIO_BASICS_HTML "FEBio_Basics.html"
#define UNITS_IN_FEBIO_HTML "Units_in_FEBio.html"
#define RUNNING_FEBIO_HTML "Running_FEBio.html"
#define RUNNING_FEBIO_ON_WINDOWS_HTML "Running_FEBio_on_Windows.html"
#define WINDOWS_XP_HTML "Windows_XP.html"
#define WINDOWS_7_HTML "Windows_7.html"
#define RUNNING_FEBIO_FROM_EXPLORER_HTML "Running_FEBio_from_Explorer.html"
#define RUNNING_FEBIO_ON_LINUX_OR_MAC_HTML "Running_FEBio_on_Linux_or_MAC.html"
#define THE_COMMAND_LINE_HTML "The_Command_Line.html"
#define THE_FEBIO_PROMPT_HTML "The_FEBio_Prompt.html"
#define THE_CONFIGURATION_FILE_HTML "The_Configuration_File.html"
#define USING_MULTIPLE_PROCESSORS_HTML "Using_Multiple_Processors.html"
#define FEBIO_OUTPUT_HTML "FEBio_Output.html"
#define SCREEN_OUTPUT_HTML "Screen_output.html"
#define OUTPUT_FILES_HTML "Output_files.html"
#define ADVANCED_OPTIONS_HTML "Advanced_Options.html"
#define INTERRUPTING_A_RUN_HTML "Interrupting_a_Run.html"
#define DEBUGGING_A_RUN_HTML "Debugging_a_Run.html"
#define SETTING_BREAK_POINTS_HTML "Setting_break_points.html"
#define RESTARTING_A_RUN_HTML "Restarting_a_Run.html"
#define FREE_FORMAT_INPUT_HTML "Free_Format_Input.html"
#define FREE_FORMAT_OVERVIEW_HTML "Free_Format_Overview.html"
#define FORMAT_SPECIFICATION_VERSIONS_HTML "Format_Specification_Versions.html"
#define NOTES_ON_BACKWARD_COMPATIBILITY_HTML "Notes_on_backward_compatibility.html"
#define MULTIPLE_INPUT_FILES_HTML "Multiple_Input_Files.html"
#define INCLUDE_KEYWORD_HTML "Include_Keyword.html"
#define THE_FROM_ATTRIBUTE_HTML "The_from_Attribute.html"
#define MODULE_SECTION_HTML "Module_Section.html"
#define CONTROL_SECTION_HTML "Control_Section.html"
#define CONTROL_PARAMETERS_HTML "Control_Parameters.html"
#define TIME_STEPPER_PARAMETERS_HTML "Time_Stepper_parameters.html"
#define COMMON_SOLVER_PARAMETERS_HTML "Common_Solver_Parameters.html"
#define SOLVER_PARAMETERS_FOR_A_STRUCTURAL_MECHANICS_ANALYSIS_HTML "Solver_Parameters_for_a_Structural_Mechanics_Analysis.html"
#define SOLVER_PARAMETERS_FOR_BIPHASIC_ANALYSIS_HTML "Solver_Parameters_for_Biphasic_Analysis.html"
#define SOLVER_PARAMETERS_FOR_SOLUTE_AND_MULTIPHASIC_ANALYSES_HTML "Solver_Parameters_for_Solute_and_Multiphasic_Analyses.html"
#define SOLVER_PARAMETERS_FOR_HEAT_ANALYSIS_HTML "Solver_Parameters_for_Heat_Analysis.html"
#define SOLVER_PARAMETERS_FOR_FLUID_AND_FLUID_FSI_ANALYSES_HTML "Solver_Parameters_for_Fluid_and_Fluid_FSI_Analyses.html"
#define GLOBALS_SECTION_HTML "Globals_Section.html"
#define CONSTANTS_HTML "Constants.html"
#define SOLUTES_HTML "Solutes.html"
#define SOLID_BOUND_MOLECULES_HTML "Solid_Bound_Molecules.html"
#define MATERIAL_SECTION_HTML "Material_Section.html"
#define MESH_SECTION_HTML "Mesh_Section.html"
#define NODES_SECTION_HTML "Nodes_Section.html"
#define ELEMENTS_SECTION_HTML "Elements_Section.html"
#define SOLID_ELEMENTS_HTML "Solid_Elements.html"
#define SHELL_ELEMENTS_HTML "Shell_Elements.html"
#define NODESET_SECTION_HTML "NodeSet_Section.html"
#define EDGE_SECTION_HTML "Edge_Section.html"
#define SURFACE_SECTION_HTML "Surface_Section.html"
#define ELEMENTSET_SECTION_HTML "ElementSet_Section.html"
#define DISCRETESET_SECTION_HTML "DiscreteSet_Section.html"
#define SURFACEPAIR_SECTION_HTML "SurfacePair_Section.html"
#define MESHDOMAINS_SECTION_HTML "MeshDomains_Section.html"
#define SOLIDDOMAIN_SECTION_HTML "SolidDomain_Section.html"
#define SHELLDOMAIN_SECTION_HTML "ShellDomain_Section.html"
#define MESHDATA_SECTION_HTML "MeshData_Section.html"
#define DATA_GENERATORS_HTML "Data_Generators.html"
#define ELEMENTDATA_HTML "ElementData.html"
#define SURFACEDATA_HTML "SurfaceData.html"
#define EDGEDATA_HTML "EdgeData.html"
#define NODEDATA_HTML "NodeData.html"
#define INITIAL_SECTION_HTML "Initial_Section.html"
#define THE_PRESTRAIN_INITIAL_CONDITION_HTML "The_Prestrain_Initial_Condition.html"
#define BOUNDARY_SECTION_HTML "Boundary_Section.html"
#define PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML "Prescribed_Nodal_Degrees_of_Freedom.html"
#define FIXED_NODAL_DEGREES_OF_FREEDOM_HTML "Fixed_Nodal_Degrees_of_Freedom.html"
#define RIGID_NODES_HTML "Rigid_Nodes.html"
#define LINEAR_CONSTRAINTS_HTML "Linear_Constraints.html"
#define RIGID_SECTION_HTML "Rigid_Section.html"
#define RIGID_CONSTRAINTS_HTML "Rigid_Constraints.html"
#define FIX_RIGID_CONSTRAINT_HTML "Fix_Rigid_Constraint.html"
#define PRESCRIBE_RIGID_CONSTRAINT_HTML "Prescribe_Rigid_Constraint.html"
#define FORCE_RIGID_CONSTRAINT_HTML "Force_Rigid_Constraint.html"
#define INITIAL_RIGID_VELOCITY_HTML "Initial_Rigid_Velocity.html"
#define INITIAL_RIGID_ANGULAR_VELOCITY_HTML "Initial_Rigid_Angular_Velocity.html"
#define RIGID_CONNECTORS_HTML "Rigid_Connectors.html"
#define RIGID_SPHERICAL_JOINT_HTML "Rigid_Spherical_Joint.html"
#define RIGID_REVOLUTE_JOINT_HTML "Rigid_Revolute_Joint.html"
#define RIGID_PRISMATIC_JOINT_HTML "Rigid_Prismatic_Joint.html"
#define RIGID_CYLINDRICAL_JOINT_HTML "Rigid_Cylindrical_Joint.html"
#define RIGID_PLANAR_JOINT_HTML "Rigid_Planar_Joint.html"
#define RIGID_LOCK_JOINT_HTML "Rigid_Lock_Joint.html"
#define RIGID_SPRING_HTML "Rigid_Spring.html"
#define RIGID_DAMPER_HTML "Rigid_Damper.html"
#define RIGID_ANGULAR_DAMPER_HTML "Rigid_Angular_Damper.html"
#define RIGID_CONTRACTILE_FORCE_HTML "Rigid_Contractile_Force.html"
#define LOADS_SECTION_HTML "Loads_Section.html"
#define NODAL_LOADS_HTML "Nodal_Loads.html"
#define NODAL_LOAD_HTML "nodal_load.html"
#define NODAL_FORCE_HTML "nodal_force.html"
#define SURFACE_LOADS_HTML "Surface_Loads.html"
#define PRESSURE_LOAD_HTML "Pressure_Load.html"
#define TRACTION_LOAD_HTML "Traction_Load.html"
#define MIXTURE_NORMAL_TRACTION_HTML "Mixture_Normal_Traction.html"
#define FLUID_FLUX_HTML "Fluid_Flux.html"
#define SOLUTE_FLUX_HTML "Solute_Flux.html"
#define HEAT_FLUX_HTML "Heat_Flux.html"
#define CONVECTIVE_HEAT_FLUX_HTML "Convective_Heat_Flux.html"
#define FLUID_TRACTION_HTML "Fluid_Traction.html"
#define FLUID_PRESSURE_HTML "Fluid_Pressure.html"
#define FLUID_NORMAL_TRACTION_HTML "Fluid_Normal_Traction.html"
#define FLUID_BACKFLOW_STABILIZATION_HTML "Fluid_Backflow_Stabilization.html"
#define FLUID_TANGENTIAL_STABILIZATION_HTML "Fluid_Tangential_Stabilization.html"
#define FLUID_NORMAL_VELOCITY_HTML "Fluid_Normal_Velocity.html"
#define FLUID_VELOCITY_HTML "Fluid_Velocity.html"
#define FLUID_ROTATIONAL_VELOCITY_HTML "Fluid_Rotational_Velocity.html"
#define FLUID_RESISTANCE_HTML "Fluid_Resistance.html"
#define FLUID_FSI_TRACTION_HTML "Fluid_FSI_Traction.html"
#define BIPHASIC_FSI_TRACTION_HTML "Biphasic_FSI_Traction.html"
#define BODY_LOADS_HTML "Body_Loads.html"
#define CONSTANT_BODY_FORCE_HTML "Constant_Body_Force.html"
#define NON_CONSTANT_BODY_FORCE_HTML "Non_Constant_Body_Force.html"
#define CENTRIFUGAL_BODY_FORCE_HTML "Centrifugal_Body_Force.html"
#define HEAT_SOURCE_HTML "Heat_Source.html"
#define SURFACE_ATTRACTION_HTML "Surface_Attraction.html"
#define CONTACT_SECTION_HTML "Contact_Section.html"
#define SLIDING_INTERFACES_HTML "Sliding_Interfaces.html"
#define BIPHASIC_CONTACT_HTML "Biphasic_Contact.html"
#define BIPHASIC_SOLUTE_AND_MULTIPHASIC_CONTACT_HTML "Biphasic_Solute_and_Multiphasic_Contact.html"
#define RIGID_WALL_INTERFACES_HTML "Rigid_Wall_Interfaces.html"
#define TIED_INTERFACES_HTML "Tied_Interfaces.html"
#define TIED_ELASTIC_INTERFACES_HTML "Tied_Elastic_Interfaces.html"
#define TIED_BIPHASIC_INTERFACES_HTML "Tied_Biphasic_Interfaces.html"
#define TIED_MULTIPHASIC_INTERFACES_HTML "Tied_Multiphasic_Interfaces.html"
#define STICKY_INTERFACES_HTML "Sticky_Interfaces.html"
#define CONSTRAINTS_SECTION_HTML "Constraints_Section.html"
#define SYMMETRY_PLANE_HTML "Symmetry_Plane.html"
#define NORMAL_FLUID_VELOCITY_CONSTRAINT_HTML "Normal_Fluid_Velocity_Constraint.html"
#define THE_PRESTRAIN_UPDATE_RULES_HTML "The_Prestrain_Update_Rules.html"
#define USING_UPDATE_RULES_HTML "Using_Update_rules.html"
#define PRESTRAIN_UPDATE_RULE_HTML "prestrain_update_rule.html"
#define THE_IN_SITU_STRETCH_UPDATE_RULE_HTML "The_in_situ_stretch_update_rule.html"
#define DISCRETE_SECTION_HTML "Discrete_Section.html"
#define DISCRETE_MATERIALS_HTML "Discrete_Materials.html"
#define LINEAR_SPRING_HTML "Linear_Spring.html"
#define NONLINEAR_SPRING_HTML "Nonlinear_spring.html"
#define HILL_HTML "Hill.html"
#define DISCRETE_SECTION_DISCRETE_SECTION_HTML "Discrete_Section_Discrete_Section.html"
#define RIGID_CABLE_HTML "Rigid_Cable.html"
#define STEP_SECTION_HTML "Step_Section.html"
#define LOADDATA_SECTION_HTML "LoadData_Section.html"
#define THE_LOADCURVE_CONTROLLER_HTML "The_loadcurve_controller.html"
#define THE_MATH_CONTROLLER_HTML "The_math_controller.html"
#define THE_PID_CONTROLLER_HTML "The_PID_controller.html"
#define OUTPUT_SECTION_HTML "Output_Section.html"
#define LOGFILE_HTML "Logfile.html"
#define NODE_DATA_CLASS_HTML "Node_Data_Class.html"
#define FACE_DATA_CLASS_HTML "Face_Data_Class.html"
#define ELEMENT_DATA_CLASS_HTML "Element_Data_Class.html"
#define RIGID_BODY_DATA_CLASS_HTML "Rigid_Body_Data_Class.html"
#define RIGID_CONNECTOR_DATA_CLASS_HTML "Rigid_Connector_Data_Class.html"
#define PLOTFILE_HTML "Plotfile.html"
#define PLOTFILE_VARIABLES_HTML "Plotfile_Variables.html"
#define MATERIALS_HTML "Materials.html"
#define ELASTIC_SOLIDS_HTML "Elastic_Solids.html"
#define SPECIFYING_FIBER_ORIENTATION_OR_MATERIAL_AXES_HTML "Specifying_Fiber_Orientation_or_Material_Axes.html"
#define TRANSVERSELY_ISOTROPIC_MATERIALS_HTML "Transversely_Isotropic_Materials.html"
#define ORTHOTROPIC_MATERIALS_HTML "Orthotropic_Materials.html"
#define UNCOUPLED_MATERIALS_HTML "Uncoupled_Materials.html"
#define ARRUDA_BOYCE_HTML "Arruda_Boyce.html"
#define ELLIPSOIDAL_FIBER_DISTRIBUTION_UNCOUPLED_HTML "Ellipsoidal_Fiber_Distribution_Uncoupled.html"
#define ELLIPSOIDAL_FIBER_DISTRIBUTION_MOONEY_RIVLIN_HTML "Ellipsoidal_Fiber_Distribution_Mooney_Rivlin.html"
#define ELLIPSOIDAL_FIBER_DISTRIBUTION_VERONDA_WESTMANN_HTML "Ellipsoidal_Fiber_Distribution_Veronda_Westmann.html"
#define FUNG_ORTHOTROPIC_HTML "Fung_Orthotropic.html"
#define HOLZAPFEL_GASSER_OGDEN_HTML "Holzapfel_Gasser_Ogden.html"
#define MOONEY_RIVLIN_HTML "Mooney_Rivlin.html"
#define MUSCLE_MATERIAL_HTML "Muscle_Material.html"
#define OGDEN_HTML "Ogden.html"
#define TENDON_MATERIAL_HTML "Tendon_Material.html"
#define TENSION_COMPRESSION_NONLINEAR_ORTHOTROPIC_HTML "Tension_Compression_Nonlinear_Orthotropic.html"
#define TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML "Transversely_Isotropic_Mooney_Rivlin.html"
#define TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML "Transversely_Isotropic_Veronda_Westmann.html"
#define UNCOUPLED_SOLID_MIXTURE_HTML "Uncoupled_Solid_Mixture.html"
#define VERONDA_WESTMANN_HTML "Veronda_Westmann.html"
#define MOONEY_RIVLIN_VON_MISES_DISTRIBUTED_FIBERS_HTML "Mooney_Rivlin_Von_Mises_Distributed_Fibers.html"
#define UNCONSTRAINED_MATERIALS_HTML "Unconstrained_Materials.html"
#define CARTER_HAYES_HTML "Carter_Hayes.html"
#define CELL_GROWTH_HTML "Cell_Growth.html"
#define CUBIC_CLE_HTML "Cubic_CLE.html"
#define DONNAN_EQUILIBRIUM_SWELLING_HTML "Donnan_Equilibrium_Swelling.html"
#define ELLIPSOIDAL_FIBER_DISTRIBUTION_HTML "Ellipsoidal_Fiber_Distribution.html"
#define ELLIPSOIDAL_FIBER_DISTRIBUTION_NEO_HOOKEAN_HTML "Ellipsoidal_Fiber_Distribution_Neo_Hookean.html"
#define ELLIPSOIDAL_FIBER_DISTRIBUTION_WITH_DONNAN_EQUILIBRIUM_SWELLING_HTML "Ellipsoidal_Fiber_Distribution_with_Donnan_Equilibrium_Swelling.html"
#define FUNG_ORTHOTROPIC_COMPRESSIBLE_HTML "Fung_Orthotropic_Compressible.html"
#define HOLMES_MOW_HTML "Holmes_Mow.html"
#define HOLZAPFEL_GASSER_OGDEN_UNCONSTRAINED_HTML "Holzapfel_Gasser_Ogden_Unconstrained.html"
#define ISOTROPIC_ELASTIC_HTML "Isotropic_Elastic.html"
#define ORTHOTROPIC_ELASTIC_HTML "Orthotropic_Elastic.html"
#define ORTHOTROPIC_CLE_HTML "Orthotropic_CLE.html"
#define OSMOTIC_PRESSURE_FROM_VIRIAL_EXPANSION_HTML "Osmotic_Pressure_from_Virial_Expansion.html"
#define NATURAL_NEO_HOOKEAN_HTML "Natural_Neo_Hookean.html"
#define NEO_HOOKEAN_HTML "Neo_Hookean.html"
#define COUPLED_MOONEY_RIVLIN_HTML "Coupled_Mooney_Rivlin.html"
#define COUPLED_VERONDA_WESTMANN_HTML "Coupled_Veronda_Westmann.html"
#define OGDEN_UNCONSTRAINED_HTML "Ogden_Unconstrained.html"
#define PERFECT_OSMOMETER_EQUILIBRIUM_OSMOTIC_PRESSURE_HTML "Perfect_Osmometer_Equilibrium_Osmotic_Pressure.html"
#define POROUS_NEO_HOOKEAN_HTML "Porous_Neo_Hookean.html"
#define SOLID_MIXTURE_HTML "Solid_Mixture.html"
#define SPHERICAL_FIBER_DISTRIBUTION_HTML "Spherical_Fiber_Distribution.html"
#define SPHERICAL_FIBER_DISTRIBUTION_FROM_SOLID_BOUND_MOLECULE_HTML "Spherical_Fiber_Distribution_from_Solid_Bound_Molecule.html"
#define COUPLED_TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML "Coupled_Transversely_Isotropic_Mooney_Rivlin.html"
#define COUPLED_TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML "Coupled_Transversely_Isotropic_Veronda_Westmann.html"
#define LARGE_POISSONS_RATIO_LIGAMENT_HTML "Large_Poissons_Ratio_Ligament.html"
#define FIBERS_HTML "Fibers.html"
#define UNCONSTRAINED_FIBER_MODELS_HTML "Unconstrained_Fiber_Models.html"
#define FIBER_WITH_EXPONENTIAL_POWER_LAW_HTML "Fiber_with_Exponential_Power_Law.html"
#define FIBER_WITH_NEO_HOOKEAN_LAW_HTML "Fiber_with_Neo_Hookean_Law.html"
#define FIBER_WITH_TOE_LINEAR_RESPONSE_HTML "Fiber_with_Toe_Linear_Response.html"
#define UNCOUPLED_FIBER_MODELS_HTML "Uncoupled_Fiber_Models.html"
#define FIBER_WITH_EXPONENTIAL_POWER_LAW_UNCOUPLED_FORMULATION_HTML "Fiber_with_Exponential_Power_Law_Uncoupled_Formulation.html"
#define FIBER_WITH_NEO_HOOKEAN_LAW_UNCOUPLED_HTML "Fiber_with_Neo_Hookean_Law_Uncoupled.html"
#define FIBER_WITH_TOE_LINEAR_RESPONSE_UNCOUPLED_FORMULATION_HTML "Fiber_with_Toe_Linear_Response_Uncoupled_Formulation.html"
#define CONTINUOUS_FIBER_DISTRIBUTION_HTML "Continuous_Fiber_Distribution.html"
#define UNCONSTRAINED_CONTINUOUS_FIBER_DISTRIBUTION_HTML "Unconstrained_Continuous_Fiber_Distribution.html"
#define UNCOUPLED_CONTINUOUS_FIBER_DISTRIBUTION_HTML "Uncoupled_Continuous_Fiber_Distribution.html"
#define DISTRIBUTION_HTML "Distribution.html"
#define SPHERICAL_HTML "Spherical.html"
#define ELLIPSOIDAL_HTML "Ellipsoidal.html"
#define PERIODIC_VON_MISES_DISTRIBUTION_HTML "Periodic_von_Mises_Distribution.html"
#define CIRCULAR_HTML "Circular.html"
#define ELLIPTICAL_HTML "Elliptical.html"
#define VON_MISES_DISTRIBUTION_HTML "von_Mises_Distribution.html"
#define SCHEME_HTML "Scheme.html"
#define GAUSS_KRONROD_TRAPEZOIDAL_RULE_HTML "Gauss_Kronrod_Trapezoidal_Rule.html"
#define FINITE_ELEMENT_INTEGRATION_RULE_HTML "Finite_Element_Integration_Rule.html"
#define TRAPEZOIDAL_RULE_HTML "Trapezoidal_Rule.html"
#define VISCOELASTIC_SOLIDS_HTML "Viscoelastic_Solids.html"
#define UNCOUPLED_VISCOELASTIC_MATERIALS_HTML "Uncoupled_Viscoelastic_Materials.html"
#define UNCONSTRAINED_VISCOELASTIC_MATERIALS_HTML "Unconstrained_Viscoelastic_Materials.html"
#define REACTIVE_VISCOELASTIC_SOLID_HTML "Reactive_Viscoelastic_Solid.html"
#define RELAXATION_FUNCTIONS_HTML "Relaxation_Functions.html"
#define EXPONENTIAL_HTML "Exponential.html"
#define EXPONENTIAL_DISTORTIONAL_HTML "Exponential_Distortional.html"
#define FUNG_HTML "Fung.html"
#define PARK_HTML "Park.html"
#define PARK_DISTORTIONAL_HTML "Park_Distortional.html"
#define POWER_HTML "Power.html"
#define POWER_DISTORTIONAL_HTML "Power_Distortional.html"
#define REACTIVE_DAMAGE_MECHANICS_HTML "Reactive_Damage_Mechanics.html"
#define GENERAL_SPECIFICATION_OF_DAMAGE_MATERIALS_HTML "General_Specification_of_Damage_Materials.html"
#define CUMULATIVE_DISTRIBUTION_FUNCTIONS_HTML "Cumulative_Distribution_Functions.html"
#define SIMO_HTML "Simo.html"
#define LOG_NORMAL_HTML "Log_Normal.html"
#define WEIBULL_HTML "Weibull.html"
#define QUINTIC_POLYNOMIAL_HTML "Quintic_Polynomial.html"
#define STEP_HTML "Step.html"
#define DAMAGE_CRITERION_HTML "Damage_Criterion.html"
#define DAMAGE_CRITERION_SIMO_HTML "Damage_Criterion_Simo.html"
#define STRAIN_ENERGY_DENSITY_HTML "Strain_Energy_Density.html"
#define SPECIFIC_STRAIN_ENERGY_HTML "Specific_Strain_Energy.html"
#define VON_MISES_STRESS_HTML "Von_Mises_Stress.html"
#define MAXIMUM_SHEAR_STRESS_HTML "Maximum_Shear_Stress.html"
#define MAXIMUM_NORMAL_STRESS_HTML "Maximum_Normal_Stress.html"
#define MAXIMUM_NORMAL_LAGRANGE_STRAIN_HTML "Maximum_Normal_Lagrange_Strain.html"
#define MULTIGENERATION_SOLIDS_HTML "Multigeneration_Solids.html"
#define GENERAL_SPECIFICATION_OF_MULTIGENERATION_SOLIDS_HTML "General_Specification_of_Multigeneration_Solids.html"
#define BIPHASIC_MATERIALS_HTML "Biphasic_Materials.html"
#define GENERAL_SPECIFICATION_OF_BIPHASIC_MATERIALS_HTML "General_Specification_of_Biphasic_Materials.html"
#define PERMEABILITY_MATERIALS_HTML "Permeability_Materials.html"
#define CONSTANT_ISOTROPIC_PERMEABILITY_HTML "Constant_Isotropic_Permeability.html"
#define EXPONENTIAL_ISOTROPIC_PERMEABILITY_HTML "Exponential_Isotropic_Permeability.html"
#define PERMEABILITY_MATERIALS_HOLMES_MOW_HTML "Permeability_Materials_Holmes_Mow.html"
#define REFERENTIALLY_ISOTROPIC_PERMEABILITY_HTML "Referentially_Isotropic_Permeability.html"
#define REFERENTIALLY_ORTHOTROPIC_PERMEABILITY_HTML "Referentially_Orthotropic_Permeability.html"
#define REFERENTIALLY_TRANSVERSELY_ISOTROPIC_PERMEABILITY_HTML "Referentially_Transversely_Isotropic_Permeability.html"
#define FLUID_SUPPLY_MATERIALS_HTML "Fluid_Supply_Materials.html"
#define STARLING_EQUATION_HTML "Starling_Equation.html"
#define BIPHASIC_SOLUTE_MATERIALS_HTML "Biphasic_Solute_Materials.html"
#define GUIDELINES_FOR_BIPHASIC_SOLUTE_ANALYSES_HTML "Guidelines_for_Biphasic_Solute_Analyses.html"
#define PRESCRIBED_BOUNDARY_CONDITIONS_HTML "Prescribed_Boundary_Conditions.html"
#define PRESCRIBED_INITIAL_CONDITIONS_HTML "Prescribed_Initial_Conditions.html"
#define GENERAL_SPECIFICATION_OF_BIPHASIC_SOLUTE_MATERIALS_HTML "General_Specification_of_Biphasic_Solute_Materials.html"
#define DIFFUSIVITY_MATERIALS_HTML "Diffusivity_Materials.html"
#define CONSTANT_ISOTROPIC_DIFFUSIVITY_HTML "Constant_Isotropic_Diffusivity.html"
#define CONSTANT_ORTHOTROPIC_DIFFUSIVITY_HTML "Constant_Orthotropic_Diffusivity.html"
#define REFERENTIALLY_ISOTROPIC_DIFFUSIVITY_HTML "Referentially_Isotropic_Diffusivity.html"
#define REFERENTIALLY_ORTHOTROPIC_DIFFUSIVITY_HTML "Referentially_Orthotropic_Diffusivity.html"
#define ALBRO_ISOTROPIC_DIFFUSIVITY_HTML "Albro_Isotropic_Diffusivity.html"
#define SOLUBILITY_MATERIALS_HTML "Solubility_Materials.html"
#define CONSTANT_SOLUBILITY_HTML "Constant_Solubility.html"
#define OSMOTIC_COEFFICIENT_MATERIALS_HTML "Osmotic_Coefficient_Materials.html"
#define CONSTANT_OSMOTIC_COEFFICIENT_HTML "Constant_Osmotic_Coefficient.html"
#define TRIPHASIC_AND_MULTIPHASIC_MATERIALS_HTML "Triphasic_and_Multiphasic_Materials.html"
#define GUIDELINES_FOR_MULTIPHASIC_ANALYSES_HTML "Guidelines_for_Multiphasic_Analyses.html"
#define INITIAL_STATE_OF_SWELLING_HTML "Initial_State_of_Swelling.html"
#define GUIDELINES_FOR_MULTIPHASIC_ANALYSES_PRESCRIBED_BOUNDARY_CONDITIONS_HTML "Guidelines_for_Multiphasic_Analyses_Prescribed_Boundary_Conditions.html"
#define GUIDELINES_FOR_MULTIPHASIC_ANALYSES_PRESCRIBED_INITIAL_CONDITIONS_HTML "Guidelines_for_Multiphasic_Analyses_Prescribed_Initial_Conditions.html"
#define PRESCRIBED_EFFECTIVE_SOLUTE_FLUX_HTML "Prescribed_Effective_Solute_Flux.html"
#define PRESCRIBED_ELECTRIC_CURRENT_DENSITY_HTML "Prescribed_Electric_Current_Density.html"
#define ELECTRICAL_GROUNDING_HTML "Electrical_Grounding.html"
#define GENERAL_SPECIFICATION_OF_MULTIPHASIC_MATERIALS_HTML "General_Specification_of_Multiphasic_Materials.html"
#define SOLVENT_SUPPLY_MATERIALS_HTML "Solvent_Supply_Materials.html"
#define SOLVENT_SUPPLY_MATERIALS_STARLING_EQUATION_HTML "Solvent_Supply_Materials_Starling_Equation.html"
#define CHEMICAL_REACTIONS_HTML "Chemical_Reactions.html"
#define GUIDELINES_FOR_CHEMICAL_REACTION_ANALYSES_HTML "Guidelines_for_Chemical_Reaction_Analyses.html"
#define GENERAL_SPECIFICATION_FOR_CHEMICAL_REACTIONS_HTML "General_Specification_for_Chemical_Reactions.html"
#define CHEMICAL_REACTION_MATERIALS_HTML "Chemical_Reaction_Materials.html"
#define LAW_OF_MASS_ACTION_FOR_FORWARD_REACTIONS_HTML "Law_of_Mass_Action_for_Forward_Reactions.html"
#define LAW_OF_MASS_ACTION_FOR_REVERSIBLE_REACTIONS_HTML "Law_of_Mass_Action_for_Reversible_Reactions.html"
#define MICHAELIS_MENTEN_REACTION_HTML "Michaelis_Menten_Reaction.html"
#define SPECIFIC_REACTION_RATE_MATERIALS_HTML "Specific_Reaction_Rate_Materials.html"
#define CONSTANT_REACTION_RATE_HTML "Constant_Reaction_Rate.html"
#define HUISKES_REACTION_RATE_HTML "Huiskes_Reaction_Rate.html"
#define RIGID_BODY_HTML "Rigid_Body.html"
#define ACTIVE_CONTRACTION_HTML "Active_Contraction.html"
#define CONTRACTION_IN_MIXTURES_OF_UNCOUPLED_MATERIALS_HTML "Contraction_in_Mixtures_of_Uncoupled_Materials.html"
#define UNCOUPLED_PRESCRIBED_UNIAXIAL_ACTIVE_CONTRACTION_HTML "Uncoupled_Prescribed_Uniaxial_Active_Contraction.html"
#define UNCOUPLED_PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_HTML "Uncoupled_Prescribed_Transversely_Isotropic_Active.html"
#define UNCOUPLED_PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_CONTRACTION_HTML "Uncoupled_Prescribed_Transversely_Isotropic_Active_Contraction.html"
#define UNCOUPLED_PRESCRIBED_ISOTROPIC_ACTIVE_CONTRACTION_HTML "Uncoupled_Prescribed_Isotropic_Active_Contraction.html"
#define PRESCRIBED_FIBER_STRESS_HTML "Prescribed_Fiber_Stress.html"
#define CONTRACTION_IN_MIXTURES_OF_UNCONSTRAINED_MATERIALS_HTML "Contraction_in_Mixtures_of_Unconstrained_Materials.html"
#define PRESCRIBED_UNIAXIAL_ACTIVE_CONTRACTION_HTML "Prescribed_Uniaxial_Active_Contraction.html"
#define PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_CONTRACTION_HTML "Prescribed_Transversely_Isotropic_Active_Contraction.html"
#define PRESCRIBED_ISOTROPIC_ACTIVE_CONTRACTION_HTML "Prescribed_Isotropic_Active_Contraction.html"
#define CONTRACTION_IN_MIXTURES_OF_UNCONSTRAINED_MATERIALS_PRESCRIBED_FIBER_STRESS_HTML "Contraction_in_Mixtures_of_Unconstrained_Materials_Prescribed_Fiber_Stress.html"
#define VISCOUS_FLUIDS_HTML "Viscous_Fluids.html"
#define GENERAL_SPECIFICATION_OF_FLUID_MATERIALS_HTML "General_Specification_of_Fluid_Materials.html"
#define VISCOUS_FLUID_MATERIALS_HTML "Viscous_Fluid_Materials.html"
#define NEWTONIAN_FLUID_HTML "Newtonian_Fluid.html"
#define CARREAU_MODEL_HTML "Carreau_Model.html"
#define CARREAU_YASUDA_MODEL_HTML "Carreau_Yasuda_Model.html"
#define POWELL_EYRING_MODEL_HTML "Powell_Eyring_Model.html"
#define CROSS_MODEL_HTML "Cross_Model.html"
#define GENERAL_SPECIFICATION_OF_FLUID_FSI_MATERIALS_HTML "General_Specification_of_Fluid_FSI_Materials.html"
#define GENERAL_SPECIFICATION_OF_BIPHASIC_FSI_MATERIALS_HTML "General_Specification_of_Biphasic_FSI_Materials.html"
#define PRESTRAIN_MATERIAL_HTML "Prestrain_material.html"
#define PRESTRAIN_MATERIAL_INTRODUCTION_HTML "Prestrain_material_Introduction.html"
#define THE_PRESTRAIN_MATERIAL_HTML "The_Prestrain_Material.html"
#define PRESTRAIN_GRADIENT_HTML "prestrain_gradient.html"
#define IN_SITU_STRETCH_HTML "in_situ_stretch.html"
#define CONTINUOUS_DAMAGE_HTML "Continuous_Damage.html"
#define DAMAGE_FIBER_POWER_HTML "Damage_Fiber_Power.html"
#define DAMAGE_FIBER_EXPONENTIAL_HTML "Damage_Fiber_Exponential.html"
#define RESTART_INPUT_FILE_HTML "Restart_Input_file.html"
#define RESTART_INPUT_FILE_INTRODUCTION_HTML "Restart_Input_file_Introduction.html"
#define THE_ARCHIVE_SECTION_HTML "The_Archive_Section.html"
#define THE_CONTROL_SECTION_HTML "The_Control_Section.html"
#define THE_LOADDATA_SECTION_HTML "The_LoadData_Section.html"
#define THE_STEP_SECTION_HTML "The_Step_Section.html"
#define EXAMPLE_HTML "Example.html"
#define EXAMPLE_1_HTML "Example_1.html"
#define EXAMPLE_2_HTML "Example_2.html"
#define PARAMETER_OPTIMIZATION_HTML "Parameter_Optimization.html"
#define OPTIMIZATION_INPUT_FILE_HTML "Optimization_Input_File.html"
#define TASK_SECTION_HTML "Task_Section.html"
#define OPTIONS_SECTION_HTML "Options_Section.html"
#define PARAMETERS_SECTION_HTML "Parameters_Section.html"
#define OBJECTIVE_SECTION_HTML "Objective_Section.html"
#define THE_DATA_FIT_MODEL_HTML "The_data_fit_model.html"
#define THE_TARGET_MODEL_HTML "The_target_model.html"
#define THE_ELEMENT_DATA_MODEL_HTML "The_element_data_model.html"
#define THE_NODE_DATA_MODEL_HTML "The_node_data_model.html"
#define OPTIMIZATION_INPUT_FILE_CONSTRAINTS_SECTION_HTML "Optimization_Input_File_Constraints_Section.html"
#define RUNNING_A_PARAMETER_OPTIMIZATION_HTML "Running_a_Parameter_Optimization.html"
#define AN_EXAMPLE_INPUT_FILE_HTML "An_Example_Input_File.html"
#define TROUBLESHOOTING_HTML "Troubleshooting.html"
#define BEFORE_YOU_RUN_YOUR_MODEL_HTML "Before_You_Run_Your_Model.html"
#define THE_FINITE_ELEMENT_MESH_HTML "The_Finite_Element_Mesh.html"
#define BEFORE_YOU_RUN_YOUR_MODEL_MATERIALS_HTML "Before_You_Run_Your_Model_Materials.html"
#define BOUNDARY_CONDITIONS_HTML "Boundary_Conditions.html"
#define DEBUGGING_A_MODEL_HTML "Debugging_a_Model.html"
#define COMMON_ISSUES_HTML "Common_Issues.html"
#define INVERTED_ELEMENTS_HTML "Inverted_elements.html"
#define MATERIAL_INSTABILITY_HTML "Material_instability.html"
#define TIME_STEP_TOO_LARGE_HTML "Time_step_too_large.html"
#define ELEMENTS_TOO_DISTORTED_HTML "Elements_too_distorted.html"
#define SHELLS_ARE_TOO_THICK_HTML "Shells_are_too_thick.html"
#define RIGID_BODY_MODES_HTML "Rigid_body_modes.html"
#define FAILURE_TO_CONVERGE_HTML "Failure_to_converge.html"
#define NO_LOADS_APPLIED_HTML "No_loads_applied.html"
#define CONVERGENCE_TOLERANCE_TOO_TIGHT_HTML "Convergence_Tolerance_Too_Tight.html"
#define FORCING_CONVERGENCE_HTML "Forcing_convergence.html"
#define PROBLEMS_DUE_TO_CONTACT_HTML "Problems_due_to_Contact.html"
#define GUIDELINES_FOR_CONTACT_PROBLEMS_HTML "Guidelines_for_Contact_Problems.html"
#define THE_PENALTY_METHOD_HTML "The_penalty_method.html"
#define AUGMENTED_LAGRANGIAN_METHOD_HTML "Augmented_Lagrangian_Method.html"
#define INITIAL_SEPARATION_HTML "Initial_Separation.html"
#define CAUTIONARY_NOTE_FOR_STEADY_STATE_BIPHASIC_AND_MULTIPHASIC_ANALYSES_HTML "Cautionary_Note_for_Steady_State_Biphasic_and_Multiphasic_Analyses.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_INITIAL_STATE_OF_SWELLING_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses_Initial_State_of_Swelling.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_PRESCRIBED_BOUNDARY_CONDITIONS_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses_Prescribed_Boundary_Conditions.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_PRESCRIBED_INITIAL_CONDITIONS_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses_Prescribed_Initial_Conditions.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_PRESCRIBED_EFFECTIVE_SOLUTE_FLUX_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses_Prescribed_Effective_Solute_Flux.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_PRESCRIBED_ELECTRIC_CURRENT_DENSITY_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses_Prescribed_Electric_Current_Density.html"
#define TROUBLESHOOTING_GUIDELINES_FOR_MULTIPHASIC_ANALYSES_ELECTRICAL_GROUNDING_HTML "Troubleshooting_Guidelines_for_Multiphasic_Analyses_Electrical_Grounding.html"
#define GUIDELINES_FOR_FLUID_ANALYSES_HTML "Guidelines_for_Fluid_Analyses.html"
#define DEGREES_OF_FREEDOM_AND_BOUNDARY_CONDITIONS_HTML "Degrees_of_Freedom_and_Boundary_Conditions.html"
#define BIASED_MESHES_FOR_BOUNDARY_LAYERS_HTML "Biased_Meshes_for_Boundary_Layers.html"
#define COMPUTATIONAL_EFFICIENCY_BROYDENS_METHOD_HTML "Computational_Efficiency_Broydens_Method.html"
#define DYNAMIC_VERSUS_STEADY_STATE_ANALYSES_HTML "Dynamic_versus_Steady_State_Analyses.html"
#define ISOTHERMAL_COMPRESSIBLE_FLOW_VERSUS_ACOUSTICS_HTML "Isothermal_Compressible_Flow_versus_Acoustics.html"
#define FLUID_STRUCTURE_INTERACTIONS_HTML "Fluid_Structure_Interactions.html"
#define UNDERSTANDING_THE_SOLUTION_HTML "Understanding_the_Solution.html"
#define MESH_CONVERGENCE_HTML "Mesh_convergence.html"
#define CONSTRAINT_ENFORCEMENT_HTML "Constraint_enforcement.html"
#define GUIDELINES_FOR_USING_PRESTRAIN_HTML "Guidelines_for_Using_Prestrain.html"
#define LIMITATIONS_OF_FEBIO_HTML "Limitations_of_FEBio.html"
#define GEOMETRICAL_INSTABILITIES_HTML "Geometrical_instabilities.html"
#define MATERIAL_INSTABILITIES_HTML "Material_instabilities.html"
#define REMESHING_HTML "Remeshing.html"
#define FORCE_DRIVEN_PROBLEMS_HTML "Force_driven_Problems.html"
#define SOLUTIONS_OBTAINED_ON_MULTI_PROCESSOR_MACHINES_HTML "Solutions_obtained_on_Multi_processor_Machines.html"
#define WHERE_TO_GET_MORE_HELP_HTML "Where_to_Get_More_Help.html"
#define CONFIGURATION_FILE_HTML "Configuration_File.html"
#define OVERVIEW_HTML "Overview.html"
#define CONFIGURING_LINEAR_SOLVERS_HTML "Configuring_Linear_Solvers.html"
#define PARDISO_HTML "Pardiso.html"
#define SKYLINE_HTML "Skyline.html"
#define FGMRES_HTML "FGMRES.html"
#define CG_HTML "CG.html"
#define BOOMERAMG_HTML "BoomerAMG.html"
#define SCHUR_HTML "Schur.html"
#define EXAMPLES_HTML "Examples.html"
#define FEBIO_PLUGINS_HTML "FEBio_Plugins.html"
#define USING_PLUGINS_HTML "Using_Plugins.html"
#define ERROR_MESSAGES_HTML "Error_Messages.html"
#define HETEROGENEOUS_MODEL_PARAMETERS_HTML "Heterogeneous_model_parameters.html"
#define MATH_PARAMETERS_HTML "Math_parameters.html"
#define MAPPED_PARAMETERS_HTML "Mapped_parameters.html"
#define REFERENCING_PARAMETERS_HTML "Referencing_Parameters.html"
#define MATH_EXPRESSION_HTML "Math_Expression.html"
#define FUNCTIONS_HTML "Functions.html"
#define MATH_EXPRESSION_CONSTANTS_HTML "Math_Expression_Constants.html"
