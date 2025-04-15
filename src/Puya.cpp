#include "Bjorklund.hpp"
#include "Catatumbo.hpp"
#include <array>

// Número máximo de voces y longitud de secuencia
static const int NUM_VOICES_MAX = 4;
static const int MAX_SEQUENCE_LEN = 32;

// Declaración anticipada de Puya para permitir su referencia en Voice
struct Puya;

struct Voice {
    // Estado de la secuencia de patrones
    std::vector<bool> seq0;
    std::vector<bool> acc0;
    std::array<bool, MAX_SEQUENCE_LEN> sequence;
    std::array<bool, MAX_SEQUENCE_LEN> accents;

    // Parámetros con valores por defecto
    unsigned int par_k = 4;  // relleno
    unsigned int par_l = 10; // longitud del patrón 
    unsigned int par_r = 1;  // rotación
    unsigned int par_p = 1;  // relleno adicional
    unsigned int par_s = 1;  // desplazamiento
    unsigned int par_a = 3;  // acentos
  
    // Seguimiento de parámetros con inicializaciones
    unsigned int par_last = 0;
    unsigned int par_k_last = 0;
    unsigned int par_l_last = 0;
    unsigned int par_a_last = 0;

    // Estado actual con inicializaciones
    unsigned int currentStep = 0;
    unsigned int turing = 0;
    bool gateOn = false;
    bool accOn = false;
    bool calculate = false;

    // Objetos DSP para manejo de triggers y pulsos
    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger syncTrigger;
    dsp::PulseGenerator gatePulse;
    dsp::PulseGenerator accentPulse;

    // Generadores de patrones para ritmos euclidianos
    Bjorklund euclid;
    Bjorklund euclid2;

    // Reinicia todos los estados de la voz a valores iniciales
    void reset() {
        // Reinicia contadores y banderas
        currentStep = 0;
        turing = 0;
        gateOn = false;
        accOn = false;
        calculate = true;
    
        // Reinicia generadores de patrones
        euclid.reset();
        euclid2.reset();

        // Reinicia todos los objetos DSP
        clockTrigger.reset();
        resetTrigger.reset();
        syncTrigger.reset();
        gatePulse.reset();
        accentPulse.reset();

        // Limpia y reinicia todas las secuencias
        seq0.clear();
        acc0.clear();
        std::fill(sequence.begin(), sequence.end(), false);
        std::fill(accents.begin(), accents.end(), false);
    }

    // Guarda estado de la voz en JSON
    json_t* toJson() {
        json_t* voiceJ = json_object();
        if (!voiceJ) return nullptr;
        
        // Almacena todos los parámetros
        json_object_set_new(voiceJ, "par_k", json_integer(par_k));
        json_object_set_new(voiceJ, "par_l", json_integer(par_l));
        json_object_set_new(voiceJ, "par_r", json_integer(par_r));
        json_object_set_new(voiceJ, "par_p", json_integer(par_p));
        json_object_set_new(voiceJ, "par_s", json_integer(par_s));
        json_object_set_new(voiceJ, "par_a", json_integer(par_a));
        
        return voiceJ;
    }
  
    // Carga estado de la voz desde JSON
    void fromJson(json_t* voiceJ) {
        // Valida entrada
        if (!voiceJ) return;
        
        // Carga todos los parámetros si están presentes
        json_t* par_kJ = json_object_get(voiceJ, "par_k");
        if (par_kJ) par_k = json_integer_value(par_kJ);
        
        json_t* par_lJ = json_object_get(voiceJ, "par_l");
        if (par_lJ) par_l = json_integer_value(par_lJ);
        
        json_t* par_rJ = json_object_get(voiceJ, "par_r");
        if (par_rJ) par_r = json_integer_value(par_rJ);
        
        json_t* par_pJ = json_object_get(voiceJ, "par_p");
        if (par_pJ) par_p = json_integer_value(par_pJ);
        
        json_t* par_sJ = json_object_get(voiceJ, "par_s");
        if (par_sJ) par_s = json_integer_value(par_sJ);
        
        json_t* par_aJ = json_object_get(voiceJ, "par_a");
        if (par_aJ) par_a = json_integer_value(par_aJ);
    }
};

struct Puya : Module {
  // Enumeraciones para parámetros, entradas, salidas y luces
  enum ParamIds {
      K_PARAM,
      L_PARAM,
      R_PARAM,
      S_PARAM,
      P_PARAM,
      A_PARAM,
      CLK_PARAM,
      TRIG_PARAM,
      VOICE_PARAM,
      SYNC_PARAM,
      NUM_PARAMS
  };

  enum InputIds {
      K_INPUT,
      L_INPUT,
      R_INPUT,
      S_INPUT,
      A_INPUT,
      P_INPUT,
      CLK_INPUT,
      RESET_INPUT,
      RND_INPUT,
      NUM_INPUTS
  };

  enum OutputIds {
      GATE_OUTPUT,
      ACCENT_OUTPUT,
      CLK_OUTPUT,
      RESET_OUTPUT,
      NUM_OUTPUTS
  };

  enum LightIds {
      CLK_LIGHT,
      GATE_LIGHT,
      ACCENT_LIGHT,
      NUM_LIGHTS
  };

  // Modos y estilos del módulo
  enum patternStyle {
      EUCLIDEAN_PATTERN,
      RANDOM_PATTERN,
      FIBONACCI_PATTERN,
      LINEAR_PATTERN,
      CANTOR_PATTERN
  } style = EUCLIDEAN_PATTERN;

  enum gateModes {
      TRIGGER_MODE,
      GATE_MODE,
      TURING_MODE
  } gateMode = TRIGGER_MODE;

  // Gestión de voces y patrones
  std::array<Voice, NUM_VOICES_MAX> voices;
  int currentVoice = 0;
  Bjorklund euclid;
  Bjorklund euclid2;

  // Parámetros con valores por defecto
  unsigned int par_k = 4;  // relleno
  unsigned int par_l = 10; // longitud del patrón
  unsigned int par_r = 1;  // rotación
  unsigned int par_p = 1;  // relleno adicional
  unsigned int par_s = 1;  // desplazamiento
  unsigned int par_a = 3;  // acentos

  // Seguimiento de parámetros
  unsigned int par_last = 0;   // suma de verificación
  unsigned int par_k_last = 0;
  unsigned int par_l_last = 0;
  unsigned int par_a_last = 0;

  // Objetos DSP
  dsp::SchmittTrigger clockTrigger;
  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger syncTrigger;
  dsp::PulseGenerator gatePulse;
  dsp::PulseGenerator accentPulse;

  // Estado del módulo
  bool gateOn = false;
  bool accOn = false;
  bool calculate = false;
  bool from_reset = false;
  unsigned int currentStep = 0;
  unsigned int turing = 0;

  // Constructor del módulo
  Puya() {
      config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  
      // Configuración de parámetros
      configParam(K_PARAM, 0.0f, 1.0f, 0.25f, "Relleno");
      configParam(L_PARAM, 0.0f, 1.0f, 1.0f, "Longitud");
      configParam(R_PARAM, 0.0f, 1.0f, 0.0f, "Rotación");
      configParam(P_PARAM, 0.0f, 1.0f, 0.0f, "Relleno adicional");
      configParam(A_PARAM, 0.0f, 1.0f, 0.0f, "Acentos");
      configParam(S_PARAM, 0.0f, 1.0f, 0.0f, "Desplazamiento");
      configParam(VOICE_PARAM, 1.0f, 4.0f, 1.0f, "Voz");
      
      // Configuración de entradas polifónicas
      for (int i = 0; i < NUM_INPUTS; i++) {
          inputs[i].setChannels(NUM_VOICES_MAX);
      }
      
      // Configuración de salidas polifónicas
      for (int i = 0; i < NUM_OUTPUTS; i++) {
          outputs[i].setChannels(NUM_VOICES_MAX);
      }
  
      // Inicializar todas las voces
      for (int v = 0; v < NUM_VOICES_MAX; v++) {
          voices[v].reset();
          resetVoice(voices[v]);
          
          // Inicializar parámetros por defecto para cada voz
          voices[v].par_k = 4;  // relleno
          voices[v].par_l = 10; // longitud del patrón
          voices[v].par_r = 1;  // rotación
          voices[v].par_p = 1;  // relleno adicional
          voices[v].par_s = 1;  // desplazamiento
          voices[v].par_a = 3;  // acentos
          
          // Guardar estado inicial
          voices[v].par_last = voices[v].par_k + voices[v].par_l + 
                             voices[v].par_r + voices[v].par_p + 
                             voices[v].par_s + voices[v].par_a;
          voices[v].calculate = true;
      }
  
      // Configurar el botón de sync como momentáneo (0=off, 1=on)
    configParam(SYNC_PARAM, 0.0f, 1.0f, 0.0f, "Sync");
    
    // Configuración de entradas polifónicas
    for (int i = 0; i < NUM_INPUTS; i++) {
        inputs[i].setChannels(NUM_VOICES_MAX);
    }
  
      onReset();
  }
  
  unsigned int fib(unsigned int n) {
      return (n < 2) ? n : fib(n - 1) + fib(n - 2);
  }

  // Métodos de serialización JSON
  json_t* dataToJson() override {
      json_t* rootJ = json_object();
      if (!rootJ) return nullptr;

      // Guarda estado del módulo
      json_object_set_new(rootJ, "mode", json_integer(static_cast<int>(gateMode)));
      json_object_set_new(rootJ, "style", json_integer(static_cast<int>(style)));
      json_object_set_new(rootJ, "currentVoice", json_integer(currentVoice));

      // Guarda estados de las voces
      json_t* voicesJ = json_array();
      if (!voicesJ) {
          json_decref(rootJ);
          return nullptr;
      }

      for (int i = 0; i < NUM_VOICES_MAX; i++) {
          json_t* voiceJ = voices[i].toJson();
          if (voiceJ) {
              json_array_append_new(voicesJ, voiceJ);
          }
      }
      json_object_set_new(rootJ, "voices", voicesJ);

      return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
      if (!rootJ) return;

      // Carga estado del módulo
      json_t* modeJ = json_object_get(rootJ, "mode");
      if (modeJ) {
          gateMode = static_cast<gateModes>(json_integer_value(modeJ));
      }

      json_t* styleJ = json_object_get(rootJ, "style");
      if (styleJ) {
          style = static_cast<patternStyle>(json_integer_value(styleJ));
      }

      json_t* currentVoiceJ = json_object_get(rootJ, "currentVoice");
      if (currentVoiceJ) {
          currentVoice = clamp(json_integer_value(currentVoiceJ), 0, NUM_VOICES_MAX - 1);
      }

      // Carga estados de las voces
      json_t* voicesJ = json_object_get(rootJ, "voices");
      if (voicesJ) {
          size_t voiceCount = std::min(static_cast<size_t>(json_array_size(voicesJ)), 
                                     static_cast<size_t>(NUM_VOICES_MAX));
          for (size_t i = 0; i < voiceCount; i++) {
              json_t* voiceJ = json_array_get(voicesJ, i);
              if (voiceJ) {
                  voices[i].fromJson(voiceJ);
              }
          }
      }
  }

  float getParameterizedVoltage(int input_id, int voice) {
    return inputs[input_id].getPolyVoltage(voice);
  }

  void onReset() override {
    // Reiniciar estado del módulo
    gateMode = TRIGGER_MODE;
    style = EUCLIDEAN_PATTERN;
    currentVoice = 0;

    for (auto& voice : voices) {
        voice.reset();
       resetVoice(voice);
    }
  }

  void resetVoice(Voice& voice) {
    // Redimensionar secuencias si cambiaron los parámetros
    if (voice.par_l_last != voice.par_l) {
       std::fill(voice.seq0.begin(), voice.seq0.end(), false);
       voice.seq0.resize(voice.par_l + voice.par_p);
    }

    if (voice.par_k_last != voice.par_k) {
       std::fill(voice.acc0.begin(), voice.acc0.end(), false);
       voice.acc0.resize(voice.par_k);
   }

   // Generar patrón según el estilo
    switch (style) {
       case RANDOM_PATTERN:
           generateRandomPattern(voice);
           if (voice.par_a > 0) {
               generateRandomAccents(voice);
           }
           break;
       case FIBONACCI_PATTERN:
           generateFibonacciPattern(voice);
            break;
       case LINEAR_PATTERN:
           generateLinearPattern(voice);
           break;
       case EUCLIDEAN_PATTERN:
          generateEuclideanPattern(voice);
            break;
        default:
            break;
   }

   distributeAccents(voice);

    // Actualizar estado
   voice.calculate = false;
   voice.par_k_last = voice.par_k;
    voice.par_l_last = voice.par_l;
   voice.par_a_last = voice.par_a;
  }

  // Métodos de generación de patrones
  void generateRandomPattern(Voice& voice) {
   voice.seq0.resize(voice.par_l);
   std::fill(voice.seq0.begin(), voice.seq0.end(), false);
    
   unsigned int n = 0;
   unsigned int f = 0;
   while (f < voice.par_k) {
        if (random::uniform() < static_cast<float>(voice.par_k) / static_cast<float>(voice.par_l)) {
            voice.seq0.at(n % voice.par_l) = true;
           f++;
       }
       n++;
    }
  }

  void generateRandomAccents(Voice& voice) {
   voice.acc0.resize(voice.par_k);
   std::fill(voice.acc0.begin(), voice.acc0.end(), false);
    
    unsigned int n = 0;
    unsigned int nacc = 0;
    while (nacc < voice.par_a) {
        if (random::uniform() < static_cast<float>(voice.par_a) / static_cast<float>(voice.par_k)) {
            voice.acc0.at(n % voice.par_k) = true;
            nacc++;
       }
       n++;
   }
  }

  void generateFibonacciPattern(Voice& voice) {
    // Generar secuencia principal
   voice.seq0.resize(voice.par_l);
    std::fill(voice.seq0.begin(), voice.seq0.end(), false);
    for (unsigned int k = 0; k < voice.par_k; k++) {
       voice.seq0.at(fib(k) % voice.par_l) = true;
   }

   // Generar acentos
   voice.acc0.resize(voice.par_k);
   std::fill(voice.acc0.begin(), voice.acc0.end(), false);
    for (unsigned int a = 0; a < voice.par_a; a++) {
       voice.acc0.at(fib(a) % voice.par_k) = true;
   }
  }

  void generateLinearPattern(Voice& voice) {
   // Generar secuencia principal
   voice.seq0.resize(voice.par_l);
   std::fill(voice.seq0.begin(), voice.seq0.end(), false);
   for (unsigned int k = 0; k < voice.par_k; k++) {
       voice.seq0.at(voice.par_l * k / voice.par_k) = true;
   }

   // Generar acentos
   voice.acc0.resize(voice.par_k);
   std::fill(voice.acc0.begin(), voice.acc0.end(), false);
   for (unsigned int a = 0; a < voice.par_a; a++) {
       voice.acc0.at(voice.par_k * a / voice.par_a) = true;
   }
  }

  void generateEuclideanPattern(Voice& voice) {
    // Generar secuencia principal
    voice.euclid.reset();
    voice.euclid.init(voice.par_l, voice.par_k);
    voice.euclid.iter();
    voice.seq0 = voice.euclid.sequence;

    // Generar acentos si es necesario
    if (voice.par_a > 0) {
        voice.euclid2.reset();
        voice.euclid2.init(voice.par_k, voice.par_a);
        voice.euclid2.iter();
        voice.acc0 = voice.euclid2.sequence;
    }
  }

  void distributeAccents(Voice& voice) {
    unsigned int j = voice.par_k - voice.par_s;
    for (unsigned int i = 0; i < voice.seq0.size(); i++) {
        unsigned int idx = (i + voice.par_r) % (voice.par_l + voice.par_p);
        voice.sequence[idx] = voice.seq0.at(i);
        voice.accents[idx] = false;
        if (voice.par_a && voice.seq0.at(i)) {
            voice.accents[idx] = voice.acc0.at(j % voice.par_k);
            j++;
        }
    }
  }

    // Métodos de procesamiento y actualización
    void process(const ProcessArgs& args) override {
      
        // Procesamiento del botón de sync al inicio
      if (syncTrigger.process(params[SYNC_PARAM].getValue())) {
         // Cuando se presiona el botón, resetear todas las voces
          for (auto& voice : voices) {
             voice.currentStep = 0;  // Establecer directamente en 0 en lugar de par_l + par_p
             voice.reset();
             resetVoice(voice);

             // Verificar si hay un hit en el primer paso y emitir el pulso
             if (voice.sequence[0]) {  // Si hay un hit en el primer paso
                 voice.gatePulse.trigger(1e-3f);  // Trigger del pulso
                 if (gateMode == GATE_MODE) {
                     voice.gateOn = true;
                 }
                 
                 // Si hay acento en el primer paso, también triggerear el acento
                 if (voice.accents[0]) {
                     voice.accentPulse.trigger(1e-3f);
                     if (gateMode == GATE_MODE) {
                            voice.accOn = true;
                        }
                    }
               }
           }
        }
        // Obtener y validar voz actual para UI
      int newVoice = clamp(static_cast<int>(params[VOICE_PARAM].getValue()) - 1, 
                         0, NUM_VOICES_MAX - 1);
      
      // Si cambió la voz seleccionada, actualizar UI y guardar estado
      if (newVoice != currentVoice) {
          saveVoiceState(voices[currentVoice]);
          currentVoice = newVoice;
          loadVoiceState(voices[currentVoice]);
      }
  
      // Procesar cada voz independientemente
      for (int v = 0; v < NUM_VOICES_MAX; v++) {
          Voice& voice = voices[v];
          bool isCurrentVoice = (v == currentVoice);
  
          // Procesar reset para esta voz específica
          if (inputs[RESET_INPUT].isConnected()) {
              if (voice.resetTrigger.process(inputs[RESET_INPUT].getVoltage(v))) {
                  voice.currentStep = voice.par_l + voice.par_p;
                  voice.reset();  // Asegurar reset completo
              }
          }
  
          // Procesar clock para esta voz específica
          bool nextStep = false;
          if (inputs[CLK_INPUT].isConnected()) {
              if (voice.clockTrigger.process(inputs[CLK_INPUT].getVoltage(v))) {
                  nextStep = true;
              }
          }
  
          if (nextStep) {
              processStep(voice);
              
              // Actualizar salidas de clock
              outputs[CLK_OUTPUT].setVoltage(10.0f, v);
          } else {
              outputs[CLK_OUTPUT].setVoltage(0.0f, v);
          }
  
          // Procesar pulsos específicos de esta voz
          bool gpulse = voice.gatePulse.process(1.0f / args.sampleRate);
          bool apulse = voice.accentPulse.process(1.0f / args.sampleRate);
  
          // Configurar salidas específicas para esta voz
          if (gateMode == TURING_MODE) {
              float turingVoltage = 10.0f * (voice.turing / std::pow(2.0f, voice.par_l) - 1.0f);
              outputs[GATE_OUTPUT].setVoltage(turingVoltage, v);
          } else {
              float gateVoltage = (voice.gateOn || gpulse) ? 10.0f : 0.0f;
              outputs[GATE_OUTPUT].setVoltage(gateVoltage, v);
          }
          
          float accentVoltage = (voice.accOn || apulse) ? 10.0f : 0.0f;
          outputs[ACCENT_OUTPUT].setVoltage(accentVoltage, v);
  
          // Solo actualizar parámetros para la voz actual en UI
          if (isCurrentVoice) {
              updateVoiceParameters(voice);
          }
      }
  
      // Asegurar que todas las salidas mantengan su configuración polifónica
      for (int i = 0; i < NUM_OUTPUTS; i++) {
          outputs[i].setChannels(NUM_VOICES_MAX);
      }

      updateLights(args);
    }

  void processStep(Voice& voice) {
      // Actualizar paso actual
      voice.currentStep++;
      if (voice.currentStep >= voice.par_l + voice.par_p) {
          voice.currentStep = 0;
      }

      // Procesar según modo
      if (gateMode == TURING_MODE) {
          voice.turing = 0;
          for (unsigned int i = 0; i < voice.par_l; i++) {
              voice.turing |= voice.sequence[(voice.currentStep + i) % (voice.par_l + voice.par_p)];
              voice.turing <<= 1;
          }
      } else {
          voice.gateOn = false;
          if (voice.sequence[voice.currentStep]) {
              voice.gatePulse.trigger(1e-3f);
              if (gateMode == GATE_MODE) {
                  voice.gateOn = true;
              }
          }
      }

      // Procesar acentos
      voice.accOn = false;
      if (voice.par_a && voice.accents.at(voice.currentStep)) {
          voice.accentPulse.trigger(1e-3f);
          if (gateMode == GATE_MODE) {
              voice.accOn = true;
          }
      }
  }

  void updateVoiceParameters(Voice& voice) {
    // Guardar estado anterior para comparación
    unsigned int oldParamSum = voice.par_l + voice.par_r + voice.par_a + 
                             voice.par_k + voice.par_p + voice.par_s;
    
    // Guardar valores anteriores
    voice.par_k_last = voice.par_k;
    voice.par_l_last = voice.par_l;
    voice.par_a_last = voice.par_a;

    // Calcular parámetros de longitud y relleno
    voice.par_l = static_cast<unsigned int>(1.0f + 15.0f * 
        clamp(params[L_PARAM].getValue() + getParameterizedVoltage(L_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_p = static_cast<unsigned int>((32.0f - voice.par_l) * 
        clamp(params[P_PARAM].getValue() + getParameterizedVoltage(P_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));

    // Calcular rotación y relleno principal
    voice.par_r = static_cast<unsigned int>((voice.par_l + voice.par_p - 1.0f) * 
        clamp(params[R_PARAM].getValue() + getParameterizedVoltage(R_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_k = static_cast<unsigned int>(1.0f + (voice.par_l - 1.0f) * 
        clamp(params[K_PARAM].getValue() + getParameterizedVoltage(K_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));

    // Calcular acentos y desplazamiento
    voice.par_a = static_cast<unsigned int>(voice.par_k * 
        clamp(params[A_PARAM].getValue() + getParameterizedVoltage(A_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));

    if (voice.par_a == 0) {
        voice.par_s = 0;
    } else {
        voice.par_s = static_cast<unsigned int>((voice.par_k - 1.0f) * 
            clamp(params[S_PARAM].getValue() + getParameterizedVoltage(S_INPUT, currentVoice) / 9.0f, 
                  0.0f, 1.0f));
    }

    // Verificar cambios
    unsigned int newParamSum = voice.par_l + voice.par_r + voice.par_a + 
                           voice.par_k + voice.par_p + voice.par_s;
                           
    if (newParamSum != oldParamSum) {
        voice.par_last = newParamSum;
        voice.calculate = true;
        saveVoiceState(voice);  // Guardar estado antes de regenerar
        resetVoice(voice);      // Regenerar patrón
    }

    // Procesar entrada de CV aleatorio
    if (inputs[RND_INPUT].isConnected()) {
        float rndCV = getParameterizedVoltage(RND_INPUT, currentVoice) / 10.0f;  // Normalizar a 0-1
        if (rndCV > 0.0f) {
          // Aplicar aleatoriedad a los parámetros proporcionalmente al voltaje
          float randomAmount = rndCV; // 0-1 basado en el voltaje de entrada
          
          // Modificar todos los parámetros aleatoriamente
         voice.par_k = static_cast<unsigned int>(1.0f + (voice.par_l - 1.0f) * 
             (params[K_PARAM].getValue() * (1.0f - randomAmount) + random::uniform() * randomAmount));
  
         voice.par_l = static_cast<unsigned int>(1.0f + 15.0f * 
              (params[L_PARAM].getValue() * (1.0f - randomAmount) + random::uniform() * randomAmount));
  
          voice.par_r = static_cast<unsigned int>((voice.par_l + voice.par_p - 1.0f) * 
              (params[R_PARAM].getValue() * (1.0f - randomAmount) + random::uniform() * randomAmount));
  
          voice.par_p = static_cast<unsigned int>((32.0f - voice.par_l) * 
              (params[P_PARAM].getValue() * (1.0f - randomAmount) + random::uniform() * randomAmount));
  
         voice.par_a = static_cast<unsigned int>(voice.par_k * 
             (params[A_PARAM].getValue() * (1.0f - randomAmount) + random::uniform() * randomAmount));
  
         voice.par_s = static_cast<unsigned int>((voice.par_k - 1.0f) * 
             (params[S_PARAM].getValue() * (1.0f - randomAmount) + random::uniform() * randomAmount));
  
          voice.calculate = true;
        }
    }
  }

  void updateLights(const ProcessArgs& args) {
    const float lightDecayRate = 10.0f;
    float deltaTime = args.sampleTime;

    Voice& voice = voices[currentVoice];
    bool gpulse = voice.gatePulse.process(1.0f / args.sampleRate);
    bool apulse = voice.accentPulse.process(1.0f / args.sampleRate);

    // Actualizar brillo de las luces
    bool clkActive = inputs[CLK_INPUT].getVoltage() > 0.0f;
    float clkBrightness = lights[CLK_LIGHT].getBrightness();
    lights[CLK_LIGHT].setBrightness(
        clamp(clkActive ? 1.0f : clkBrightness - deltaTime * lightDecayRate, 0.0f, 1.0f));

    bool gateActive = voice.gateOn || gpulse;
    float gateBrightness = lights[GATE_LIGHT].getBrightness();
    lights[GATE_LIGHT].setBrightness(
        clamp(gateActive ? 1.0f : gateBrightness - deltaTime * lightDecayRate, 0.0f, 1.0f));

    bool accentActive = voice.accOn || apulse;
    float accentBrightness = lights[ACCENT_LIGHT].getBrightness();
    lights[ACCENT_LIGHT].setBrightness(
        clamp(accentActive ? 1.0f : accentBrightness - deltaTime * lightDecayRate, 0.0f, 1.0f));
 }

  void saveVoiceState(Voice& voice) {
    // Guardar todos los parámetros principales
    voice.par_k = static_cast<unsigned int>(1.0f + (voice.par_l - 1.0f) * 
        clamp(params[K_PARAM].getValue() + getParameterizedVoltage(K_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_l = static_cast<unsigned int>(1.0f + 15.0f * 
        clamp(params[L_PARAM].getValue() + getParameterizedVoltage(L_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_r = static_cast<unsigned int>((voice.par_l + voice.par_p - 1.0f) * 
        clamp(params[R_PARAM].getValue() + getParameterizedVoltage(R_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_p = static_cast<unsigned int>((32.0f - voice.par_l) * 
        clamp(params[P_PARAM].getValue() + getParameterizedVoltage(P_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_a = static_cast<unsigned int>(voice.par_k * 
        clamp(params[A_PARAM].getValue() + getParameterizedVoltage(A_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));
    voice.par_s = static_cast<unsigned int>((voice.par_k - 1.0f) * 
        clamp(params[S_PARAM].getValue() + getParameterizedVoltage(S_INPUT, currentVoice) / 9.0f, 
              0.0f, 1.0f));

    // Guardar últimos valores para comparación
    voice.par_k_last = voice.par_k;
    voice.par_l_last = voice.par_l;
    voice.par_a_last = voice.par_a;
    voice.par_last = voice.par_k + voice.par_l + voice.par_r + voice.par_p + voice.par_s + voice.par_a;
  }

  void loadVoiceState(Voice& voice) {
    // Restaurar parámetros de la voz a los controles
    params[K_PARAM].setValue(
        clamp((voice.par_k - 1.0f) / (voice.par_l - 1.0f), 0.0f, 1.0f)
    );
    params[L_PARAM].setValue(
        clamp((voice.par_l - 1.0f) / 15.0f, 0.0f, 1.0f)
    );
    params[R_PARAM].setValue(
        clamp(static_cast<float>(voice.par_r) / (voice.par_l + voice.par_p - 1.0f), 0.0f, 1.0f)
    );
    params[P_PARAM].setValue(
        clamp(static_cast<float>(voice.par_p) / (32.0f - voice.par_l), 0.0f, 1.0f)
    );
    params[A_PARAM].setValue(
        clamp(static_cast<float>(voice.par_a) / voice.par_k, 0.0f, 1.0f)
    );
    params[S_PARAM].setValue(
        clamp(static_cast<float>(voice.par_s) / (voice.par_k - 1.0f), 0.0f, 1.0f)
    );

    // Regenerar patrón si es necesario
    if (voice.par_last != (voice.par_k + voice.par_l + voice.par_r + voice.par_p + voice.par_s + voice.par_a)) {
        voice.calculate = true;
        resetVoice(voice);
    }
  }
};

// Widget de visualización para el módulo Puya
struct PuyaDisplay : TransparentWidget {
  Puya* module;
  std::shared_ptr<Font> font;
  float y1;
  float yh;

  // Colores para cada voz
  const NVGcolor voiceColors[NUM_VOICES_MAX] = {
    nvgRGB(0xff, 0xff, 0x00), // Voz 1: Amarillo
    nvgRGB(0x00, 0x00, 0xff), // Voz 2: Azul
    nvgRGB(0xff, 0x00, 0x00), // Voz 3: Rojo
    nvgRGB(0x00, 0xff, 0x00)  // Voz 4: Verde
};

  PuyaDisplay(float y1_, float yh_) {
      y1 = y1_;
      yh = yh_;
      font = APP->window->loadFont(asset::plugin(pluginInstance, "res/hdad-segment14-1.002/Segment14.ttf"));
  }

  void drawPolygon(NVGcontext* vg) {
      if (!module) return;

      int voiceIndex = clamp((int)module->params[Puya::VOICE_PARAM].getValue() - 1, 0, NUM_VOICES_MAX - 1);
      Voice& voice = module->voices[voiceIndex];
      NVGcolor voiceColor = voiceColors[voiceIndex];

      Rect b = Rect(Vec(2.0, 2.0), box.size.minus(Vec(2.0, 2.0)));
      float cx = 0.5f * b.size.x + 1.0f;
      float cy = 0.5f * b.size.y - 12.0f;
      const float r1 = 0.45f * b.size.x;
      const float r2 = 0.35f * b.size.x;

      // Círculos con el color de la voz
      nvgBeginPath(vg);
      nvgStrokeColor(vg, nvgRGBA(voiceColor.r * 127, voiceColor.g * 127, voiceColor.b * 127, 0xff));
      nvgFillColor(vg, voiceColor);
      nvgStrokeWidth(vg, 1.0f);
      nvgCircle(vg, cx, cy, r1);
      nvgCircle(vg, cx, cy, r2);
      nvgStroke(vg);

      const unsigned int len = voice.par_l + voice.par_p;

      // Dibujar la secuencia con el color de la voz
      nvgStrokeColor(vg, voiceColor);
      nvgBeginPath(vg);
      bool first = true;

      // Pasos inactivos
      for (unsigned int i = 0; i < len; i++) {
          if (!voice.sequence[i]) {
              float r = voice.accents[i] ? r1 : r2;
              float x = cx + r * std::cosf(2.0f * M_PI * i / len - 0.5f * M_PI);
              float y = cy + r * std::sinf(2.0f * M_PI * i / len - 0.5f * M_PI);

              nvgBeginPath(vg);
              nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0x00)); // Relleno transparente
              nvgStrokeWidth(vg, 1.0f);
              nvgStrokeColor(vg, nvgRGBA(voiceColor.r * 127, voiceColor.g * 127, voiceColor.b * 127, 0xff));
              nvgCircle(vg, x, y, 3.0f);
              nvgFill(vg);
              nvgStroke(vg);
          }
      }

      // Trayectoria con el color de la voz
      nvgBeginPath(vg);
      nvgStrokeColor(vg, voiceColor);
      nvgStrokeWidth(vg, 1.0f);
      for (unsigned int i = 0; i < len; i++) {
          if (voice.sequence[i]) {
              float a = static_cast<float>(i) / len;
              float r = voice.accents[i] ? r1 : r2;
              float x = cx + r * std::cosf(2.0f * M_PI * a - 0.5f * M_PI);
              float y = cy + r * std::sinf(2.0f * M_PI * a - 0.5f * M_PI);

              Vec p(x, y);
              if (voice.par_k == 1) {
                  nvgCircle(vg, x, y, 3.0f);
              }
              if (first) {
                  nvgMoveTo(vg, p.x, p.y);
                  first = false;
              } else {
                  nvgLineTo(vg, p.x, p.y);
              }
          }
      }
      nvgClosePath(vg);
      nvgStroke(vg);

      // Anillos de pasos activos con el color de la voz
      for (unsigned int i = 0; i < len; i++) {
          if (voice.sequence[i]) {
              float r = voice.accents[i] ? r1 : r2;
              float x = cx + r * std::cosf(2.0f * M_PI * i / len - 0.5f * M_PI);
              float y = cy + r * std::sinf(2.0f * M_PI * i / len - 0.5f * M_PI);

              nvgBeginPath(vg);
              nvgFillColor(vg, nvgRGBA(0x30, 0x10, 0x10, 0x00)); // Relleno transparente
              nvgStrokeWidth(vg, 1.0f);
              nvgStrokeColor(vg, voiceColor);
              nvgCircle(vg, x, y, 3.0f);
              nvgFill(vg);
              nvgStroke(vg);
          }
      }

      // Indicador del paso actual con el color de la voz
      unsigned int i = voice.currentStep;
      if (i < len) {  // Validación de índice
          float r = voice.accents[i] ? r1 : r2;
          float x = cx + r * std::cosf(2.0f * M_PI * i / len - 0.5f * M_PI);
          float y = cy + r * std::sinf(2.0f * M_PI * i / len - 0.5f * M_PI);
          nvgBeginPath(vg);
          nvgStrokeColor(vg, voiceColor);
          nvgFillColor(vg, voice.sequence[i] ? voiceColor : nvgRGBA(0x30, 0x10, 0x10, 0x00));
          nvgCircle(vg, x, y, 3.0f);
          nvgStrokeWidth(vg, 1.5f);
          nvgFill(vg);
          nvgStroke(vg);
      }
  }

  void draw(const DrawArgs& args) override {
      if (!module) return;

      // Fondo completamente transparente
      nvgBeginPath(args.vg);
      nvgRoundedRect(args.vg, 0.0f, 0.0f, box.size.x, box.size.y, 5.0f);
      nvgFillColor(args.vg, nvgRGBA(0x30, 0x10, 0x10, 0x00)); // Fondo transparente
      nvgFill(args.vg);
      nvgStrokeWidth(args.vg, 1.5f);
      nvgStrokeColor(args.vg, nvgRGBA(0xd0, 0xd0, 0xd0, 0x00)); // Borde transparente
      nvgStroke(args.vg);

      // Dibujar visualización de secuencia con el color de la voz actual
      drawPolygon(args.vg);

      // Dibujar texto de parámetros con el color de la voz
      nvgFontSize(args.vg, 8.0f);
      nvgFontFaceId(args.vg, font->handle);

      int voiceIndex = clamp((int)module->params[Puya::VOICE_PARAM].getValue() - 1, 0, NUM_VOICES_MAX - 1);
      Voice& voice = module->voices[voiceIndex];
      NVGcolor textColor = voiceColors[voiceIndex];

      Vec textPos = Vec(15.0f, 105.0f);
      nvgFillColor(args.vg, textColor);
      char str[20];
      snprintf(str, sizeof(str), "%2d %2d %2d", static_cast<int>(voice.par_k), 
              static_cast<int>(voice.par_l), static_cast<int>(voice.par_r));
      nvgText(args.vg, textPos.x, textPos.y - 11.0f, str, nullptr);

      snprintf(str, sizeof(str), "%2d %2d %2d", static_cast<int>(voice.par_p), 
              static_cast<int>(voice.par_a), static_cast<int>(voice.par_s));
      nvgText(args.vg, textPos.x, textPos.y, str, nullptr);
  }
};

// TURQUESA
struct TurquoiseLight : GrayModuleLightWidget {
  TurquoiseLight() {
      addBaseColor(nvgRGB(64, 224, 208)); // Turquesa: RGB(64, 224, 208)
  }
};

// Luz personalizada que puede cambiar de color
struct MultiColorLight : ModuleLightWidget {
  NVGcolor colors[NUM_VOICES_MAX];
  
  MultiColorLight() {
      // Inicializar con los colores de las voces
      colors[0] = nvgRGB(0xff, 0xff, 0x00); // Amarillo
      colors[1] = nvgRGB(0x00, 0x00, 0xff); // Azul
      colors[2] = nvgRGB(0xff, 0x00, 0x00); // Rojo
      colors[3] = nvgRGB(0x00, 0xff, 0x00); // Verde
      
      // Configuración inicial
      box.size = Vec(mm2px(2.176f), mm2px(2.176f));
      addBaseColor(nvgRGBA(0xff, 0xff, 0xff, 0xff));
  }

  void drawLight(const DrawArgs& args) override {
      if (!module) return;
      
      // Obtener voz actual
      int currentVoice = clamp(
          static_cast<int>(module->params[Puya::VOICE_PARAM].getValue()) - 1,
          0, NUM_VOICES_MAX - 1
      );
      
      // Dibujar luz con el color de la voz actual
      nvgBeginPath(args.vg);
      nvgCircle(args.vg, box.size.x / 2.0f, box.size.y / 2.0f, box.size.x / 2.0f);
      
      // Aplicar brillo usando firstLightId
      float brightness = module->lights[firstLightId].getBrightness();
      NVGcolor color = colors[currentVoice];
      color.a *= brightness;
      
      // Establecer color y dibujar
      nvgFillColor(args.vg, color);
      nvgFill(args.vg);
  }
};

// Widget del panel principal del módulo
struct PuyaWidget : ModuleWidget {

  PuyaWidget() = default;  // Constructor por defecto

  explicit PuyaWidget(Puya* module) {
      setModule(module);

      box.size = Vec(15.0f * 6.0f, 380.0f);

      // Panel SVG principal
      {
          auto* panel = new SvgPanel();
          panel->box.size = box.size;
          panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Puya.svg")));
          addChild(panel);
      }

      // Visualización de patrones
      {
          auto* display = new PuyaDisplay(180.0f, 30.0f);
          display->module = module;
          display->box.pos = Vec(3.0f, 30.0f);
          display->box.size = Vec(box.size.x - 6.0f, 110.0f);
          addChild(display);
      }

      // Posiciones base para controles
      const float y1 = 160.0f;
      const float yh = 30.0f;
      const float x1 = 4.0f;
      const float x2 = x1 + 30.0f;
      const float x3 = x1 + 60.0f;

      // Fila superior - Relleno, Longitud, Rotación
      addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1), module, Puya::K_PARAM));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1), module, Puya::L_PARAM));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1), module, Puya::R_PARAM));
      addInput(createInput<sp_Port>(Vec(x1, y1 + yh), module, Puya::K_INPUT));
      addInput(createInput<sp_Port>(Vec(x2, y1 + yh), module, Puya::L_INPUT));
      addInput(createInput<sp_Port>(Vec(x3, y1 + yh), module, Puya::R_INPUT));

      // Fila media - Relleno adicional, Acentos, Desplazamiento
      addParam(createParam<sp_SmallBlackKnob>(Vec(x1, y1 + 2.5f * yh), module, Puya::P_PARAM));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x2, y1 + 2.5f * yh), module, Puya::A_PARAM));
      addParam(createParam<sp_SmallBlackKnob>(Vec(x3, y1 + 2.5f * yh), module, Puya::S_PARAM));
      addInput(createInput<sp_Port>(Vec(x1, y1 + 3.5f * yh), module, Puya::P_INPUT));
      addInput(createInput<sp_Port>(Vec(x2, y1 + 3.5f * yh), module, Puya::A_INPUT));
      addInput(createInput<sp_Port>(Vec(x3, y1 + 3.5f * yh), module, Puya::S_INPUT));

      // Reloj y Reset
      addInput(createInput<PJ301MAqua>(Vec(x1, y1 + 4.65f * yh), module, Puya::CLK_INPUT));
      addInput(createInput<sp_Port>(Vec(x1, y1 + 5.4f * yh), module, Puya::RESET_INPUT));    // Sync input
      addOutput(createOutput<sp_Port>(Vec(x3, y1 + 4.65f * yh), module, Puya::CLK_OUTPUT));
      // Salidas de Gate y Accent
      addOutput(createOutput<PJ301MAqua>(Vec(x2, y1 + 4.65f * yh), module, Puya::GATE_OUTPUT));
      addOutput(createOutput<PJ301MAqua>(Vec(x2, y1 + 5.4f * yh), module, Puya::ACCENT_OUTPUT));
      
      // última actualización de controles
      addParam(createParam<SmallPB61303>(Vec(x1 - 2.0f, y1 + 6.30f * yh), module, Puya::SYNC_PARAM)); // Sync button
      addInput(createInput<sp_Port>(Vec(x2 + 1.0f, y1 + 6.15f * yh), module, Puya::RND_INPUT));    // Random CV input
      
      // Selector de voz
      addParam(createParamCentered<RoundBlackKnob>(Vec(x3 + 10.0f, y1 + 5.9f * yh), module, Puya::VOICE_PARAM));

      // Luces indicadoras con colores por voz
      addChild(createLightCentered<MultiColorLight>(Vec(5.0f, 319.0f), module, Puya::CLK_LIGHT));
      addChild(createLightCentered<MultiColorLight>(Vec(35.0f, 319.0f), module, Puya::GATE_LIGHT));
      addChild(createLightCentered<MultiColorLight>(Vec(35.0f, 340.0f), module, Puya::ACCENT_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
      if (!module) return;

      auto* puya = dynamic_cast<Puya*>(module);
      if (!puya) return;

      // Ítem del menú para modos de compuerta
      struct PuyaGateModeItem : MenuItem {
          Puya* puya = nullptr;
          Puya::gateModes gm{};

          void onAction(const event::Action& e) override {
              if (puya) puya->gateMode = gm;
          }

          void step() override {
              rightText = (puya && puya->gateMode == gm) ? "✔" : "";
              MenuItem::step();
          }
      };

      // Ítem del menú para estilos de patrón
      struct PuyaPatternStyleItem : MenuItem {
          Puya* puya = nullptr;
          Puya::patternStyle ps{};

          void onAction(const event::Action& e) override {
              if (!puya) return;
              
              puya->style = ps;
              // Reiniciar todas las voces al cambiar el estilo
              for (auto& voice : puya->voices) {
                  puya->resetVoice(voice);
              }
          }

          void step() override {
              rightText = (puya && puya->style == ps) ? "✔" : "";
              MenuItem::step();
          }
      };

      // Menú de modo de compuerta
      menu->addChild(new MenuSeparator());
      menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Modo de Compuerta"));
      menu->addChild(construct<PuyaGateModeItem>(
          &MenuItem::text, "Disparo",
          &PuyaGateModeItem::puya, puya,
          &PuyaGateModeItem::gm, Puya::TRIGGER_MODE
      ));
      menu->addChild(construct<PuyaGateModeItem>(
          &MenuItem::text, "Compuerta",
          &PuyaGateModeItem::puya, puya,
          &PuyaGateModeItem::gm, Puya::GATE_MODE
      ));
      menu->addChild(construct<PuyaGateModeItem>(
          &MenuItem::text, "Turing",
          &PuyaGateModeItem::puya, puya,
          &PuyaGateModeItem::gm, Puya::TURING_MODE
      ));

      // Menú de estilo de patrón
      menu->addChild(new MenuSeparator());
      menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Estilo de Patrón"));
      menu->addChild(construct<PuyaPatternStyleItem>(
          &MenuItem::text, "Euclides",
          &PuyaPatternStyleItem::puya, puya,
          &PuyaPatternStyleItem::ps, Puya::EUCLIDEAN_PATTERN
      ));
      menu->addChild(construct<PuyaPatternStyleItem>(
          &MenuItem::text, "Fibonacci",
          &PuyaPatternStyleItem::puya, puya,
          &PuyaPatternStyleItem::ps, Puya::FIBONACCI_PATTERN
      ));
      menu->addChild(construct<PuyaPatternStyleItem>(
          &MenuItem::text, "Aleatorio",
          &PuyaPatternStyleItem::puya, puya,
          &PuyaPatternStyleItem::ps, Puya::RANDOM_PATTERN
      ));
      menu->addChild(construct<PuyaPatternStyleItem>(
          &MenuItem::text, "Lineal",
          &PuyaPatternStyleItem::puya, puya,
          &PuyaPatternStyleItem::ps, Puya::LINEAR_PATTERN
      ));
  }
};

// Creación del modelo del módulo
Model* modelPuya = createModel<Puya, PuyaWidget>("Puya");
