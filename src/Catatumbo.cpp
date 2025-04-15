#include "Catatumbo.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
  pluginInstance = p;

  p->addModel(modelPuya);

}
