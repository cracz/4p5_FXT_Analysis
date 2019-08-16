# 4p5_FXT_Analysis
Package for analyzing 4.5 GeV Au+Au Fixed Target data (but without necessary StRoot/StPicoEvent library).

## Description of setup

* Make an `out/` directory to hold all `.out` and `.err` files resulting from the jobs. 
* `StRoot/StPicoEvent/` should be placed int the `source/` directory and compiled with the included makefile.
* Submit jobs from the top directory that contains the XML scripts. Use `cleanup.sh` to clear everything resulting from the job submission.

### Main analysis
* Output files from the main analysis will go to `results/`. 
* Use `extractPlots.sh` to automatically find the job you want to look at and then, for that job, combine all output files and print out all plots from the analysis.
  * Alternatively, these actions can be done individually using the ROOT macros `combine.cxx` and `plot.cxx`.
  * Once the output files are combined, the individual files can be deleted unless specific run numbers are needed.
  
### Patching an incomplete analysis
* In some cases the main analysis will fail for some subset of the input files and the corresponding output files will be missing from the `results/` directory.
* If this happens, and only a few files are missing, move all analyzed files to the `patchTable/`.
  * Remove all job IDs using `rename [jobID]_ "" \*` (make sure to include the underscore after the job ID and the empty quotation marks as the replacement)
  * Use `findHoles.sh` to show which files are missing so that when you submit the job again, you can use the second job submission to copy over the missing files. (If the second job submission is fully successful, just delete these files in `patchTable/`).
  * Once the file set in `patchTable/` is complete, use `prependID.sh` to add whatever job ID or identifier you want to the front of all of the files. (!!A job ID at the beginning of the files is necessary for the rest of the code to work!!)
  * Now the file set is patched and can be combined back in the `results/` directory.
  
### Integrated Yields
* To get integrated yields, use the ROOT macro `intYield.cxx` with `root -l -b -q 'intYield.cxx("[jobID]","[E||L||B]")'`
  * E, L, and B correspond to different fitting models: E = exponential, L = Levy, B = Blast-Wave.
* This macro will save the total integrated yield plot for every particle, as well as every fitted histogram with its corresponding fit function; also for every particle.
* Use `overlay.cxx` the same way as `intYield.cxx` to plot out the total raw and integrated yields overlaid in the same plots.
* To plot out every successful fit in every rapidity bin for every particle (~150 plots so be sure you want to do this):
  * load `overlay.cxx` while in a ROOT session using `.L overlay.cxx`
  * run `showAllFits("[jobID]","[E||L||B]")`


