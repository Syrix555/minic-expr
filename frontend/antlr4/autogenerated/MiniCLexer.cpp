
// Generated from MiniC.g4 by ANTLR 4.12.0


#include "MiniCLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct MiniCLexerStaticData final {
  MiniCLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  MiniCLexerStaticData(const MiniCLexerStaticData&) = delete;
  MiniCLexerStaticData(MiniCLexerStaticData&&) = delete;
  MiniCLexerStaticData& operator=(const MiniCLexerStaticData&) = delete;
  MiniCLexerStaticData& operator=(MiniCLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag miniclexerLexerOnceFlag;
MiniCLexerStaticData *miniclexerLexerStaticData = nullptr;

void miniclexerLexerInitialize() {
  assert(miniclexerLexerStaticData == nullptr);
  auto staticData = std::make_unique<MiniCLexerStaticData>(
    std::vector<std::string>{
      "T_L_PAREN", "T_R_PAREN", "T_SEMICOLON", "T_L_BRACE", "T_R_BRACE", 
      "T_ASSIGN", "T_COMMA", "T_ADD", "T_SUB", "T_MUL", "T_DIV", "T_MOD", 
      "T_LT", "T_GT", "T_LE", "T_GE", "T_EQ", "T_NEQ", "T_RETURN", "T_INT", 
      "T_VOID", "T_IF", "T_ELSE", "T_ID", "T_DIGIT", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
    },
    std::vector<std::string>{
      "", "'('", "')'", "';'", "'{'", "'}'", "'='", "','", "'+'", "'-'", 
      "'*'", "'/'", "'%'", "'<'", "'>'", "'<='", "'>='", "'=='", "'!='", 
      "'return'", "'int'", "'void'", "'if'", "'else'"
    },
    std::vector<std::string>{
      "", "T_L_PAREN", "T_R_PAREN", "T_SEMICOLON", "T_L_BRACE", "T_R_BRACE", 
      "T_ASSIGN", "T_COMMA", "T_ADD", "T_SUB", "T_MUL", "T_DIV", "T_MOD", 
      "T_LT", "T_GT", "T_LE", "T_GE", "T_EQ", "T_NEQ", "T_RETURN", "T_INT", 
      "T_VOID", "T_IF", "T_ELSE", "T_ID", "T_DIGIT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,0,26,154,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,1,0,1,0,1,1,1,1,1,2,1,2,
  	1,3,1,3,1,4,1,4,1,5,1,5,1,6,1,6,1,7,1,7,1,8,1,8,1,9,1,9,1,10,1,10,1,11,
  	1,11,1,12,1,12,1,13,1,13,1,14,1,14,1,14,1,15,1,15,1,15,1,16,1,16,1,16,
  	1,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,19,1,19,1,19,1,19,
  	1,20,1,20,1,20,1,20,1,20,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,23,
  	1,23,5,23,120,8,23,10,23,12,23,123,9,23,1,24,1,24,1,24,4,24,128,8,24,
  	11,24,12,24,129,1,24,1,24,4,24,134,8,24,11,24,12,24,135,1,24,1,24,5,24,
  	140,8,24,10,24,12,24,143,9,24,1,24,3,24,146,8,24,1,25,4,25,149,8,25,11,
  	25,12,25,150,1,25,1,25,0,0,26,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,
  	19,10,21,11,23,12,25,13,27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,
  	21,43,22,45,23,47,24,49,25,51,26,1,0,8,3,0,65,90,95,95,97,122,4,0,48,
  	57,65,90,95,95,97,122,2,0,88,88,120,120,3,0,48,57,65,70,97,102,1,0,48,
  	55,1,0,49,57,1,0,48,57,3,0,9,10,13,13,32,32,161,0,1,1,0,0,0,0,3,1,0,0,
  	0,0,5,1,0,0,0,0,7,1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,
  	1,0,0,0,0,17,1,0,0,0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,
  	0,0,0,27,1,0,0,0,0,29,1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,
  	0,37,1,0,0,0,0,39,1,0,0,0,0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,47,
  	1,0,0,0,0,49,1,0,0,0,0,51,1,0,0,0,1,53,1,0,0,0,3,55,1,0,0,0,5,57,1,0,
  	0,0,7,59,1,0,0,0,9,61,1,0,0,0,11,63,1,0,0,0,13,65,1,0,0,0,15,67,1,0,0,
  	0,17,69,1,0,0,0,19,71,1,0,0,0,21,73,1,0,0,0,23,75,1,0,0,0,25,77,1,0,0,
  	0,27,79,1,0,0,0,29,81,1,0,0,0,31,84,1,0,0,0,33,87,1,0,0,0,35,90,1,0,0,
  	0,37,93,1,0,0,0,39,100,1,0,0,0,41,104,1,0,0,0,43,109,1,0,0,0,45,112,1,
  	0,0,0,47,117,1,0,0,0,49,145,1,0,0,0,51,148,1,0,0,0,53,54,5,40,0,0,54,
  	2,1,0,0,0,55,56,5,41,0,0,56,4,1,0,0,0,57,58,5,59,0,0,58,6,1,0,0,0,59,
  	60,5,123,0,0,60,8,1,0,0,0,61,62,5,125,0,0,62,10,1,0,0,0,63,64,5,61,0,
  	0,64,12,1,0,0,0,65,66,5,44,0,0,66,14,1,0,0,0,67,68,5,43,0,0,68,16,1,0,
  	0,0,69,70,5,45,0,0,70,18,1,0,0,0,71,72,5,42,0,0,72,20,1,0,0,0,73,74,5,
  	47,0,0,74,22,1,0,0,0,75,76,5,37,0,0,76,24,1,0,0,0,77,78,5,60,0,0,78,26,
  	1,0,0,0,79,80,5,62,0,0,80,28,1,0,0,0,81,82,5,60,0,0,82,83,5,61,0,0,83,
  	30,1,0,0,0,84,85,5,62,0,0,85,86,5,61,0,0,86,32,1,0,0,0,87,88,5,61,0,0,
  	88,89,5,61,0,0,89,34,1,0,0,0,90,91,5,33,0,0,91,92,5,61,0,0,92,36,1,0,
  	0,0,93,94,5,114,0,0,94,95,5,101,0,0,95,96,5,116,0,0,96,97,5,117,0,0,97,
  	98,5,114,0,0,98,99,5,110,0,0,99,38,1,0,0,0,100,101,5,105,0,0,101,102,
  	5,110,0,0,102,103,5,116,0,0,103,40,1,0,0,0,104,105,5,118,0,0,105,106,
  	5,111,0,0,106,107,5,105,0,0,107,108,5,100,0,0,108,42,1,0,0,0,109,110,
  	5,105,0,0,110,111,5,102,0,0,111,44,1,0,0,0,112,113,5,101,0,0,113,114,
  	5,108,0,0,114,115,5,115,0,0,115,116,5,101,0,0,116,46,1,0,0,0,117,121,
  	7,0,0,0,118,120,7,1,0,0,119,118,1,0,0,0,120,123,1,0,0,0,121,119,1,0,0,
  	0,121,122,1,0,0,0,122,48,1,0,0,0,123,121,1,0,0,0,124,125,5,48,0,0,125,
  	127,7,2,0,0,126,128,7,3,0,0,127,126,1,0,0,0,128,129,1,0,0,0,129,127,1,
  	0,0,0,129,130,1,0,0,0,130,146,1,0,0,0,131,133,5,48,0,0,132,134,7,4,0,
  	0,133,132,1,0,0,0,134,135,1,0,0,0,135,133,1,0,0,0,135,136,1,0,0,0,136,
  	146,1,0,0,0,137,141,7,5,0,0,138,140,7,6,0,0,139,138,1,0,0,0,140,143,1,
  	0,0,0,141,139,1,0,0,0,141,142,1,0,0,0,142,146,1,0,0,0,143,141,1,0,0,0,
  	144,146,5,48,0,0,145,124,1,0,0,0,145,131,1,0,0,0,145,137,1,0,0,0,145,
  	144,1,0,0,0,146,50,1,0,0,0,147,149,7,7,0,0,148,147,1,0,0,0,149,150,1,
  	0,0,0,150,148,1,0,0,0,150,151,1,0,0,0,151,152,1,0,0,0,152,153,6,25,0,
  	0,153,52,1,0,0,0,7,0,121,129,135,141,145,150,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  miniclexerLexerStaticData = staticData.release();
}

}

MiniCLexer::MiniCLexer(CharStream *input) : Lexer(input) {
  MiniCLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *miniclexerLexerStaticData->atn, miniclexerLexerStaticData->decisionToDFA, miniclexerLexerStaticData->sharedContextCache);
}

MiniCLexer::~MiniCLexer() {
  delete _interpreter;
}

std::string MiniCLexer::getGrammarFileName() const {
  return "MiniC.g4";
}

const std::vector<std::string>& MiniCLexer::getRuleNames() const {
  return miniclexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& MiniCLexer::getChannelNames() const {
  return miniclexerLexerStaticData->channelNames;
}

const std::vector<std::string>& MiniCLexer::getModeNames() const {
  return miniclexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& MiniCLexer::getVocabulary() const {
  return miniclexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView MiniCLexer::getSerializedATN() const {
  return miniclexerLexerStaticData->serializedATN;
}

const atn::ATN& MiniCLexer::getATN() const {
  return *miniclexerLexerStaticData->atn;
}




void MiniCLexer::initialize() {
  ::antlr4::internal::call_once(miniclexerLexerOnceFlag, miniclexerLexerInitialize);
}
