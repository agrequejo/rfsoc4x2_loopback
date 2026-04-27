source hw_bd.tcl

set script_dir [file dirname [file normalize [info script]]]
set project_dir [file dirname [file dirname [file normalize [info script]]]]
set project_name "hw"

open_bd_design $project_name/${project_name}.srcs/sources_1/bd/design_1/design_1.bd
regenerate_bd_layout
validate_bd_design
update_compile_order -fileset sources_1
save_bd_design
launch_runs synth_1 -jobs 4
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1

write_hw_platform -fixed -include_bit -force -file sw/design_1_wrapper.xsa

# close_project
# exit