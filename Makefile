CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

SRCS = main.cpp decoder/decoder.cpp tokenizer/tokenizer.cpp
OBJS = $(SRCS:.cpp=.o)

main: $(OBJS)
	$(CXX) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) main

.PHONY: clean
