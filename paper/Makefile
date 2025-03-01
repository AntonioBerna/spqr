COMPILER=lualatex
OUTPUT_PDF=spqr
INPUT_FILE=main.tex

all:
	$(COMPILER) -jobname=$(OUTPUT_PDF) $(INPUT_FILE)
	$(COMPILER) -jobname=$(OUTPUT_PDF) $(INPUT_FILE)

clean:
	$(RM) *.aux *.log *.out *.fls *.fdb_latexmk *.toc

clean_pdf:
	$(RM) $(OUTPUT_PDF).pdf

.PHONY: all clean clean_pdf
