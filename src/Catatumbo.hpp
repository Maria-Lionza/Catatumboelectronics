#include "rack.hpp"

using namespace rack;

extern Plugin *pluginInstance;

extern Model *modelPuya;

// GUI COMPONENTS

struct PJ301MAqua : app::SvgPort {
  PJ301MAqua() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PJ301MAqua.svg")));
  }
};

struct SmallPB61303 : PB61303 {
  SmallPB61303() {
      // Original PB61303 size reducido a la mitad
      box.size = Vec(mm2px(12.2f), mm2px(13.8f));
  }
};

struct sp_Port : app::SvgPort {
  sp_Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-Port20.svg")));
  }
};

struct sp_Switch : app::SvgSwitch {
  sp_Switch() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-switchv_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-switchv_1.svg")));
  }
};

struct sp_Encoder : app::SvgKnob {
  sp_Encoder() {
    minAngle = -1.0f * M_PI;
    maxAngle = 1.0f * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-encoder.svg")));
    //sw->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-encoder.svg"));
    //sw->wrap();
    //box.size = sw->box.size;
  }
};

struct sp_BlackKnob : app::SvgKnob {
  sp_BlackKnob() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-knobBlack-large.svg")));
    box.size = Vec(30, 30);
  }
};

struct sp_SmallBlackKnob : app::SvgKnob {
  sp_SmallBlackKnob() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-knobBlack.svg")));
    //sw->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-knobBlack.svg"));
    //sw->wrap();
    //box.size = Vec(20,20);
  }
};

struct sp_Trimpot : app::SvgKnob {
  sp_Trimpot() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-trimpotBlack.svg")));
    //sw->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/sp-knobBlack.svg"));
    //sw->wrap();
    //box.size = Vec(18,18);
  }
};
