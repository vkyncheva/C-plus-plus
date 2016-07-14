# A C++ and ROOT based tool for Noise Scan Analysis

Tool for noise scans plots and comparisons
- Plots the Rate Vs HV graphs for given scan
- Fits the graphs in their HV range
- Comapare any two scans given the reference for their HV working point
- - Takes the rate at the WP from the fit for both scans per Roll unit
- - Produces plots on rate differences distributions, grouped on detector geometry basis
- The tool is easyly extended to be adapted for any X vs Y variables studies(the current for example), given it's data oriented
