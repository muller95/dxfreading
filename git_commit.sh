commit_comment=$1
echo $commit_comment
exec `git add cross_check_funcs.c`
exec `git add cross_check_funcs.h`
exec `git add dxf_geometry.c`
exec `git add dxf_geometry.h`
exec `git add dxf_nest.c`
exec `git add dxf_work_functions.c`
exec `git add dxf_work_functions.h`
exec `git add main.c`
exec `git add Makefile`
exec `git add nestapi_core.c`
exec `git add nestapi_core.h`
exec `git add nestapi_core_structs.h`
exec `git add nfp_nest_test.c`
exec `git add nfp_nest.c`

