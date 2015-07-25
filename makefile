.SUFFIXES:

CXX := g++
CXXFLAGS := -std=c++0x -Wall -Wextra -Werror -Wno-missing-field-initializers

debug: CXXFLAGS += -DDEBUG -g -O0

OBJDIR := obj
SRCDIR := .
INCLUDEDIR := .

MKDIR_P = mkdir -p

SHARED_SRC := shared.cpp
SHARED_DEP := shared.h

RECEIVER_SRC := receiver.cpp
RECEIVER_DEP := receiver.h

SENDER_SRC := sender.cpp
SENDER_DEP := sender.h

GBN_RECEIVER := gbnReceiver
GBN_RECEIVER_SRC := receiver_gbn.cpp

GBN_SENDER := gbnSender
GBN_SENDER_SRC := sender_gbn.cpp

SR_RECEIVER := srReceiver
SR_RECEIVER_SRC := receiver_sr.cpp

SR_SENDER := srSender
SR_SENDER_SRC := sender_sr.cpp

SHARED_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SHARED_SRC))
RECEIVER_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(RECEIVER_SRC))
SENDER_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SENDER_SRC))
GBN_RECEIVER_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(GBN_RECEIVER_SRC))
GBN_SENDER_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(GBN_SENDER_SRC))
SR_RECEIVER_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SR_RECEIVER_SRC))
SR_SENDER_OBJ := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SR_SENDER_SRC))

TEMP_FILES := .DS_Store dest recvInfo channelInfo

.PHONY: all directories clean

all: directories $(GBN_RECEIVER) $(GBN_SENDER) $(SR_RECEIVER) $(SR_SENDER)

debug: all

directories: $(OBJDIR)

$(OBJDIR):
	$(MKDIR_P) $(OBJDIR)

$(GBN_RECEIVER): $(GBN_RECEIVER_OBJ) $(RECEIVER_OBJ) $(SHARED_OBJ)
	$(CXX) -o $@ $^

$(GBN_SENDER): $(GBN_SENDER_OBJ) $(SENDER_OBJ) $(SHARED_OBJ)
	$(CXX) -o $@ $^

$(SR_RECEIVER): $(SR_RECEIVER_OBJ) $(RECEIVER_OBJ) $(SHARED_OBJ)
	$(CXX) -o $@ $^

$(SR_SENDER): $(SR_SENDER_OBJ) $(SENDER_OBJ) $(SHARED_OBJ)
	$(CXX) -o $@ $^

clean:
	rm -rf $(OBJDIR) $(GBN_RECEIVER) $(GBN_SENDER) $(SR_RECEIVER) $(SR_SENDER) $(TEMP_FILES)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -c $< -o $@
