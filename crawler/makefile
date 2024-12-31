# Makefile

# Compiler and flags
CXX = g++
CXXFLAGS = `xml2-config --cflags --libs` -lcurl -lpython3.10
INCLUDES = -I/home/lbp400/.local/lib/python3.10/site-packages/pybind11/include -I/usr/include/python3.10

# Target executable and source files
TARGET = crawler
SRC = crawler.cpp

# If you have multiple source files, you can specify them in the SRC variable, like so:

# makefile
# Copy code
# SRC = main.cpp helper.cpp utils.cpp

# Rule to build the target
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(INCLUDES)

# Clean up command to remove the executable
clean:
	rm -f $(TARGET)
