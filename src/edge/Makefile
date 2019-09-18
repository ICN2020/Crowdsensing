OS = $(shell uname -s)

INSTDIR=/usr/local
INSTINCDIR=$(INSTDIR)/include
INSTLIBDIR=$(INSTDIR)/lib
INSTBINDIR=$(INSTDIR)/bin

ifeq ($(OS), Darwin)
CXX=clang++
else
CXX=g++
endif

OPENCVCFLAGS=`pkg-config opencv4 --cflags`
OPENCVLDFLAGS=`pkg-config opencv4 --libs`

ifeq ($(OS), Linux)
OPENCVCFLAGS=-I/usr/local/include/opencv4
OPENCVLDFLAGS=-L/usr/local/lib -lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_dnn_objdetect -lopencv_dpm -lopencv_face -lopencv_fuzzy -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_dnn -lopencv_plot -lopencv_videostab -lopencv_video -lopencv_xfeatures2d -lopencv_shape -lopencv_ml -lopencv_ximgproc -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core
endif

INCFLAGS=$(OPENCVCFLAGS)
CXXFLAGS=-std=c++14 -Wall -O2 -g $(INCFLAGS) `pkg-config json11 --cflags`
LDFLAGS=
LDLIBS=-lfcopss -lndn-cxx -lboost_system -lpthread $(OPENCVLDFLAGS) `pkg-config json11 --libs`

SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := obj
DEP_DIR := dep

SOURCES	 := $(notdir $(foreach d, $(SRC_DIR), $(wildcard $(d)/*.cpp)))
OBJECTS	 := $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))
DEPFILES := $(addprefix $(DEP_DIR)/, $(SOURCES:.cpp=.d))
LIBFILES := $(LOADLIBES) $(LDLIBS)


TARGET := $(BIN_DIR)/edge

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@[ -d $(BIN_DIR) ] || mkdir -p $(BIN_DIR)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(COMPILE.cc) $< -o $@

$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp
	@[ -d $(DEP_DIR) ] || mkdir -p $(DEP_DIR)
	@set -e; $(COMPILE.cc) -MM $(CXXFLAGS) $< | sed 's#\($*\)\.o[ :]*#$(OBJ_DIR)/\1.o $@ : #g' > $@; [ -s $@ ] || rm -f $@

clean:
	@$(RM) $(OBJECTS) $(TARGET) $(DEPS) *.bak *~ core* GTAGS GSYMS GRTAGS GPATH
	@for sd in $(SUBDIRS); do \
	  cd $$sd; \
	  $(RM) *~ core* GTAGS GSYMS GRTAGS GPATH; \
	  cd ..; \
	done

install: all
	install $(TARGET) $(INSTBINDIR)

distclean: clean
	@$(RM) -fr $(DEP_DIR) $(OBJ_DIR) $(TARGET)

$(DEP_DIR)/%.d: ;
.PRECIOUS: $(DEP_DIR)/%.d

-include $(DEPFILES)
