sort:
	@echo " Compile sort_main ...";
	gcc -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sort_main.c ./src/record.c ./src/sort.c ./src/merge.c ./src/chunk.c -lbf -lhp_file -o ./build/sort_main -O2

clean:
	@echo "Cleaning..."
	rm -f ./build/sort_main # Removes the executable
	rm -f $(CURDIR)/*.db     # Removes .db files in the directory of the Makefile
