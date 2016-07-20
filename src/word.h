//word.h
#ifndef WORD_H
#define WORD_H

#define MAX_STRING 100

#include <node.h>
#include <node_object_wrap.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define MAX_STRING 100
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000
#define MAX_CODE_LENGTH 40


const int vocab_hash_size = 30000000;
struct vocab_word {
	long long cn;
	int *point;
	char *word, *code, codelen;
};

typedef struct {
	void * id;	
	char* train_file;
	long long file_size;
	int num_threads;
	long long * word_count_actual;
	int debug_mode;
	float * alpha;
	float * starting_alpha;
	long long  train_words;
	float sample;
	struct vocab_word** vocab;
	int window;
	clock_t* start;
	int cbow;
	int negative;
	float ** syn1;
	int hs;
	float ** syn0;
	float ** syn1neg;
	float ** expTable;
	int ** table;
	int vocab_size;
	int ** vocab_hash;
	long long layer1_size;
	long long iter;
}Args;
using namespace v8;

namespace word {
	

	class Word: public node::ObjectWrap {
	public:
		static void Init(Local<Object> exports);
		static void NewInstance(const FunctionCallbackInfo<Value>& args);
	private:
		explicit Word(const FunctionCallbackInfo<Value>& args);
		~Word();
	static void New(const FunctionCallbackInfo<Value>& args);
	static void AddWord(const FunctionCallbackInfo<Value>& args);
	static void Get(const FunctionCallbackInfo<Value>& args);
	static Persistent<Function> constructor;
		char output_file[MAX_STRING];
		char train_file[MAX_STRING];
		long long layer1_size;
		char save_vocab_file[MAX_STRING];
		char read_vocab_file[MAX_STRING];
		int debug_mode;
		int binary;
		int cbow;
		float alpha;
		int window;
		float sample ;
		int hs ;
		int negative ;
		int min_count;
		int classes;
		float *expTable;
		int *vocab_hash;
		long long vocab_size, min_reduce;
		long long word_count_actual;
		struct vocab_word *vocab;
		int num_threads;
		int* table;
		float* syn1;
		clock_t start;
		long long train_words, file_size;
		long long iter;

		float *syn0, *syn1neg;
		void TrainModel();
		void ReduceVocab();
		void ReadVocab();
		//void ReadWord(char *word, FILE *fin);
		int AddWordToVocab(char *word);
		void SortVocab();
		//int GetWordHash(char *word);
		void LearnVocabFromTrainFile();
		void SaveVocab();
		//int SearchVocab(char *word);
		void InitNet();
		void InitUnigramTable();
		void  CreateBinaryTree();
		int Get(char* word, Isolate* isolate, Handle<Object> result);
		//void *TrainModelThread(void *id);

		//int VocabCompare(const void *a, const void *b);
	};
}

#endif