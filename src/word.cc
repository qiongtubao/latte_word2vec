#include "word.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <iostream>

using namespace v8;
using namespace std;

namespace word {
	const int vocab_hash_size = 30000000; 
	Persistent<Function> Word::constructor;
	
	long long vocab_max_size = 1000;
	const int table_size = 1e8;
	Word::Word(const FunctionCallbackInfo<Value>& args)  {
		Isolate* isolate = args.GetIsolate();
		Handle<Object> config = Handle<Object>::Cast(args[0]);
		
			Handle<Value> tranFileValue = config->Get(String::NewFromUtf8(isolate, "train"));
	  			
			//输入文件
			if(tranFileValue -> IsUndefined() == false) {
				String::Utf8Value tranFile(tranFileValue);
				strcpy(train_file, *tranFile);
			}else{
				train_file[0] = 0;
			}
			//输出文件
			Handle<Value> outputValue = config->Get(String::NewFromUtf8(isolate, "output"));
			if(outputValue->IsUndefined() == false) {
				String::Utf8Value output(outputValue);
	  			strcpy(output_file, *output);
			}else{
				output_file[0] = 0;
			}
			//layer1_size
			Handle<Value> sizeValue = config->Get(String::NewFromUtf8(isolate, "size"));
			if(outputValue->IsUndefined() == false) {
				layer1_size = sizeValue->NumberValue();
			}else{
				layer1_size = 100;
			}

			Handle<Value> saveVocabValue  = config->Get(String::NewFromUtf8(isolate, "save-vocab")) ;
			if(saveVocabValue->IsUndefined() == false) {
				String::Utf8Value saveVocab(saveVocabValue);
				strcpy(save_vocab_file, *saveVocab);
			}else{
				save_vocab_file[0] = 0; 
			}

			Handle<Value> readVocabValue = config->Get(String::NewFromUtf8(isolate, "read-vocab"));
			if(readVocabValue->IsUndefined() == false) {
				String::Utf8Value readVocab(readVocabValue);
				strcpy(read_vocab_file, *readVocab);
			}else{
				read_vocab_file[0] = 0;
			}

			Handle<Value> debugValue = config->Get(String::NewFromUtf8(isolate, "debug"));
			if(debugValue->IsUndefined() == false) {
				debug_mode = debugValue->NumberValue();
			}else{
				debug_mode = 2;
			}

			Handle<Value> binaryValue = config->Get(String::NewFromUtf8(isolate, "binary"));
			if(binaryValue->IsUndefined() == false) {
				binary = binaryValue->NumberValue();
			}else{
				binary = 0;
			}

			Handle<Value> cbowValue = config->Get(String::NewFromUtf8(isolate, "cbow"));
			if(cbowValue->IsUndefined() == false) {
				cbow = cbowValue->NumberValue();
			}else{
				cbow = 1;
			}

			Handle<Value> alphaValue = config->Get(String::NewFromUtf8(isolate, "alpha"));
			if(alphaValue->IsUndefined() == false) {
				alpha = alphaValue->NumberValue();
			}else{
				if (cbow) {
					alpha = 0.05;
				}else{
					alpha = 0.025;
				}
				
			}

			Handle<Value> windowValue = config->Get(String::NewFromUtf8(isolate, "window"));
			if(windowValue->IsUndefined() == false) {
				window = windowValue->NumberValue();
			}else{
				window = 5;
			}

			Handle<Value> sampleValue = config->Get(String::NewFromUtf8(isolate, "sample"));
			if(sampleValue->IsUndefined() == false) {
				sample = sampleValue->NumberValue();
			}else{
				sample = 1e-3;
			}

			Handle<Value> hsValue = config->Get(String::NewFromUtf8(isolate, "hs"));
			if(hsValue->IsUndefined() == false) {
				hs = hsValue->NumberValue();
			}else{
				hs = 0;
			}

			Handle<Value> negativeValue = config->Get(String::NewFromUtf8(isolate, "negative"));
			if(negativeValue->IsUndefined() == false) {
				negative = negativeValue->NumberValue();
			}else{
				negative = 25;
			}

			Handle<Value> minCountValue = config->Get(String::NewFromUtf8(isolate, "min-count"));
			if(minCountValue->IsUndefined() == false) {
				min_count = minCountValue->NumberValue();
			}else{
				min_count = 5;
			}

			Handle<Value> classesValue = config->Get(String::NewFromUtf8(isolate, "classes"));
			if(classesValue->IsUndefined() == false) {
				classes = classesValue->NumberValue();
			}else{
				classes = 0;
			}


			Handle<Value> threadsValue = config->Get(String::NewFromUtf8(isolate, "threads"));
			if(threadsValue->IsUndefined() == false) {
				num_threads = threadsValue->NumberValue();
			}else{
				num_threads = 12;
			}
			Handle<Value> iterValue = config->Get(String::NewFromUtf8(isolate, "iter"));
			if(iterValue->IsUndefined() == false) {
				iter = iterValue->NumberValue();

			}else{
				iter = 5;
			}

			FILE *fileCache;
			fileCache = fopen(output_file, "rb");
			if(fileCache == NULL) {
				vocab_size = 0;
				min_reduce = 1;
				train_words = 0;
				file_size = 0;
				word_count_actual = 0;
				vocab = (struct vocab_word*)calloc(vocab_max_size, sizeof(struct vocab_word));
				vocab_hash = (int*) calloc(vocab_hash_size, sizeof(int));
				expTable = (float *)malloc((EXP_TABLE_SIZE + 1) * sizeof(float));
				for(int i = 0; i < EXP_TABLE_SIZE; i++) {
					expTable[i] = exp((i/ (float)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP);
					expTable[i] = expTable[i]/ (expTable[i] + 1);
				}
				TrainModel();
			}else{

			}
			
		//-train text8 -output vectors.bin -cbow 1 -size 200 -window 8 -negative 25 -hs 0 -sample 1e-4 -threads 20 -binary 1 -iter 15
			

		//config_ = config;
	}
	Word::~Word() {

	}	
		int GetWordHash(char *word) {
			unsigned long long a, hash = 0;

		  	for (a = 0; a < strlen(word); a++) {
		  		hash = hash * 257 + word[a];
		  	};
		  	hash = hash % vocab_hash_size;

		  	return hash;
		}

			int VocabCompare(const void *a, const void *b) {
				return ((struct vocab_word *)b)->cn - ((struct vocab_word *)a)->cn;
			}
		void Word::SortVocab() {
			int a, size;
			  unsigned int hash;
			  // Sort the vocabulary and keep </s> at the first position
			  qsort(&vocab[1], vocab_size - 1, sizeof(struct vocab_word), VocabCompare);
			  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
			  size = vocab_size;
			  train_words = 0;
			  for (a = 0; a < size; a++) {
			    // Words occuring less than min_count times will be discarded from the vocab
			    if ((vocab[a].cn < min_count) && (a != 0)) {
			      vocab_size--;
			      free(vocab[a].word);
			    } else {
			      // Hash will be re-computed, as after the sorting it is not actual
			      hash=GetWordHash(vocab[a].word);
			      while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
			      vocab_hash[hash] = a;
			      train_words += vocab[a].cn;
			    }
			  }
			  vocab = (struct vocab_word *)realloc(vocab, (vocab_size + 1) * sizeof(struct vocab_word));
			  // Allocate memory for the binary tree construction
			  for (a = 0; a < vocab_size; a++) {
			    vocab[a].code = (char *)calloc(MAX_CODE_LENGTH, sizeof(char));
			    vocab[a].point = (int *)calloc(MAX_CODE_LENGTH, sizeof(int));
			  }
		}
		int Word::AddWordToVocab(char *word) {
		  unsigned int hash, length = strlen(word) + 1;
		  if (length > MAX_STRING) length = MAX_STRING;
		  vocab[vocab_size].word = (char *)calloc(length, sizeof(char));

		  strcpy(vocab[vocab_size].word, word);
		  vocab[vocab_size].cn = 0;
		  vocab_size++;
		  // Reallocate memory if needed
		 
		  if (vocab_size + 2 >= vocab_max_size) {
		    vocab_max_size += 1000;
		    vocab = (struct vocab_word *)realloc(vocab, vocab_max_size * sizeof(struct vocab_word));
		  }
		  
		   hash = GetWordHash(word);
		
		  while (vocab_hash[hash] != -1) {
		  	hash = (hash + 1) % vocab_hash_size;
		 }
		  

		  vocab_hash[hash] = vocab_size - 1;
		  return vocab_size - 1;
		}
		void ReadWord(char *word, FILE *fin) {
			int a = 0, ch;
			  while (!feof(fin)) {
			    ch = fgetc(fin);
			    if (ch == 13) continue;
			    if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
			      if (a > 0) {
			        if (ch == '\n') ungetc(ch, fin);
			        break;
			      }
			      if (ch == '\n') {
			        strcpy(word, (char *)"</s>");
			        return;
			      } else continue;
			    }
			    word[a] = ch;
			    a++;
			    if (a >= MAX_STRING - 1) a--;   // Truncate too long words
			  }
			  word[a] = 0;

		}

		void Word::ReadVocab() {
			long long a, i = 0;
			char c;
			char word[MAX_STRING];
			FILE * fin = fopen(read_vocab_file, "rb");
			if (fin == NULL) {
			    printf("Vocabulary file not found\n");
			    exit(1);
			  }
			  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
			  vocab_size = 0;
			  while (1) {
			    ReadWord(word, fin);
			    if (feof(fin)) break;
			    a = AddWordToVocab(word);
			    fscanf(fin, "%lld%c", &vocab[a].cn, &c);
			    i++;
			  }
			  SortVocab();
			  if (debug_mode > 0) {
			    printf("Vocab size: %lld\n", vocab_size);
			    printf("Words in train file: %lld\n", train_words);
			  }
			  fin = fopen(train_file, "rb");
			  if (fin == NULL) {
			    printf("ERROR: training data file not found!\n");
			    exit(1);
			  }
			  fseek(fin, 0, SEEK_END);
			  file_size = ftell(fin);
			  fclose(fin);

		}
		
		void Word::ReduceVocab() {
			int a, b = 0;
			unsigned int hash;
			for(a = 0; a < vocab_size; a++) {
				if(vocab[a].cn > min_reduce) {
					vocab[b].cn = vocab[a].cn;
					vocab[b].word = vocab[a].word;
					b++;
				}else{
					free(vocab[a].word);
				}
			}
			vocab_size = b;
			for(a = 0; a < vocab_hash_size; a++) {
				vocab_hash[a] = -1;
			}
			for(a = 0; a < vocab_size; a++) {
				hash = GetWordHash(vocab[a].word);
				while(vocab_hash[hash] != -1) {
					hash = (hash + 1) % vocab_hash_size;

				}
				vocab_hash[hash] = a;
			}
			fflush(stdout);
			min_reduce++;
		}
			int SearchVocab(char *word, struct vocab_word *vocab, int* vocab_hash) {
			  unsigned int hash = GetWordHash(word);
			  while (1) {
			    if (vocab_hash[hash] == -1) return -1;
			    if (!strcmp(word, vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
			    hash = (hash + 1) % vocab_hash_size;
			  }
			  return -1;
			}
		void Word::LearnVocabFromTrainFile() {
			char word[MAX_STRING];
			  FILE *fin;
			  long long a, i;
			  for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
			  fin = fopen(train_file, "rb");
			  if (fin == NULL) {
			    printf("ERROR: training data file not found!\n");
			    exit(1);
			  }
			  vocab_size = 0;
			  
			  AddWordToVocab((char *)"</s>");
			
			  while (1) {
			    ReadWord(word, fin);
			    if (feof(fin)) break;
			    train_words++;
			    if ((debug_mode > 1) && (train_words % 100000 == 0)) {
			      printf("%lldK%c", train_words / 1000, 13);
			      fflush(stdout);
			    }
			    i = SearchVocab(word, vocab, vocab_hash);
			    if (i == -1) {
			      a = AddWordToVocab(word);
			      vocab[a].cn = 1;
			    } else vocab[i].cn++;
			    if (vocab_size > vocab_hash_size * 0.7) ReduceVocab();
			  }
			  SortVocab();
			  if (debug_mode > 0) {
			    printf("Vocab size: %lld\n", vocab_size);
			    printf("Words in train file: %lld\n", train_words);
			  }
			  file_size = ftell(fin);
			  fclose(fin);
		}
		
		//long long layer1_size = 100;
		

		int ReadWordIndex(FILE *fin, struct vocab_word *vocab, int* vocab_hash) {
		  char word[MAX_STRING];
		  ReadWord(word, fin);
		  if (feof(fin)) return -1;
		  return SearchVocab(word,  vocab,  vocab_hash);
		}
		void *TrainModelThread(void *arg) {
			void* id = ((Args *)arg)->id;
			long long train_words = ((Args *)arg) -> train_words;
			long long file_size = ((((Args *)arg) ->file_size)); 
			long long *  word_count_actual_address = (((Args *)arg) -> word_count_actual); 
			long long word_count_actual = *word_count_actual_address;
			float sample = ((((Args *)arg)-> sample)); 
			//float starting_alpha = *((((Args *)arg)-> starting_alpha)); 
			float * alpha_address = (((Args *)arg) ->alpha);
			float * starting_alpha_address = (((Args *)arg)-> starting_alpha);
			int num_threads = (((Args *)arg)->num_threads);
			int window = (((Args *)arg) ->window);
			int debug_mode = (((Args *)arg) ->debug_mode);
			char* train_file = (((Args *)arg) ->train_file);
			
			clock_t start =  *(((Args *)arg) ->start);
			int cbow = ((Args *)arg) ->cbow;
			int negative = ((Args *)arg) ->negative;
			int vocab_size = ((Args *)arg) ->vocab_size;
			float* syn1 = *(((Args *)arg) ->syn1);
			int hs = ((Args *)arg) ->hs;
			float* syn0 = *(((Args *)arg) ->syn0);
			float* syn1neg = *(((Args *)arg) ->syn1neg);
			float* expTable = *(((Args *)arg) ->expTable);
			struct vocab_word *vocab = *(((Args *)arg) ->vocab);
			int* table = *(((Args *)arg) -> table);
			int* vocab_hash = *(((Args *)arg) -> vocab_hash);
			long long iter = (((Args *)arg) -> iter);
			long long layer1_size = (((Args *)arg) -> layer1_size);
			

			//*((long long *) word_count_actual_address) = (*word_count_actual_address)+1;
			cout<<"alpha_address:" <<alpha_address << " " << (*alpha_address)<<"\n"<<endl;
			cout<<"starting_alpha_address:" <<starting_alpha_address << " " << (*starting_alpha_address) << "\n" << endl; 
			//cout<<"middle:"<<(*word_count_actual_address)<< " " <<word_count_actual_address<<endl;
			

			
			long long a, b, d, cw, word, last_word, sentence_length = 0, sentence_position = 0;
			  long long word_count = 0, last_word_count = 0, sen[MAX_SENTENCE_LENGTH + 1];
			  long long l1, l2, c, target, label, local_iter = iter;
			  unsigned long long next_random = (long long)id;
			  float f, g;
			  clock_t now;
			  float *neu1 = (float *)calloc(layer1_size, sizeof(float));
			  float *neu1e = (float *)calloc(layer1_size, sizeof(float));
			  FILE *fi = fopen(train_file, "rb");
			  //cout<<"num_threads:"<<(long long )num_threads <<" "<< (long long )id<<endl;
			  fseek(fi, file_size / (long long)num_threads * (long long)id, SEEK_SET);
			  long long nums = 0;
			  while (1) {
			  	nums++;
			    if (word_count - last_word_count > 10000) {
			       (*word_count_actual_address) += word_count - last_word_count;
			      last_word_count = word_count;
			      if ((debug_mode > 1)) {
			        now=clock();
			        printf("%cAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", 13, *alpha_address,
			         (*word_count_actual_address) / (float)(iter * train_words + 1) * 100,
			          (*word_count_actual_address) / ((float)(now - start + 1) / (float)CLOCKS_PER_SEC * 1000));
			        fflush(stdout);
			      }
			      //cout<<starting_alpha <<"*" << "1 - "<< word_count_actual << "/(" <<iter <<"*" << train_words << "+1)"<<endl;
			      //exit(0); 
			      
			      (*alpha_address) = (*starting_alpha_address) * (1 -  (*word_count_actual_address) / (float)(iter * train_words + 1));
			      if ((*alpha_address) < (*starting_alpha_address) * 0.0001) (*alpha_address) = (*starting_alpha_address) * 0.0001;
			    }
			    if (sentence_length == 0) {
			      while (1) {
			        word = ReadWordIndex(fi, vocab, vocab_hash);
			        if (feof(fi)) break;
			        if (word == -1) continue;
			        word_count++;
			        if (word == 0) break;
			        // The subsampling randomly discards frequent words while keeping the ranking same
			        if (sample > 0) {
			          float ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) / vocab[word].cn;
			          next_random = next_random * (unsigned long long)25214903917 + 11;
			          if (ran < (next_random & 0xFFFF) / (float)65536) continue;
			        }
			        sen[sentence_length] = word;
			        sentence_length++;
			        if (sentence_length >= MAX_SENTENCE_LENGTH) break;
			      }
			      sentence_position = 0;
			    }
			    if (feof(fi) || (word_count > train_words / num_threads)) {
			      (*word_count_actual_address) += word_count - last_word_count;
			      local_iter--;
			      if (local_iter == 0) break;
			      word_count = 0;
			      last_word_count = 0;
			      sentence_length = 0;
			      fseek(fi, file_size / (long long)num_threads * (long long)id, SEEK_SET);
			      continue;
			    }
			    word = sen[sentence_position];
			    if (word == -1) continue;
			    for (c = 0; c < layer1_size; c++) neu1[c] = 0;
			    for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
			    next_random = next_random * (unsigned long long)25214903917 + 11;
			    b = next_random % window;
			    if (cbow) {  //train the cbow architecture
			      // in -> hidden
			      cw = 0;
			      for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
			        c = sentence_position - window + a;
			        if (c < 0) continue;
			        if (c >= sentence_length) continue;
			        last_word = sen[c];
			        if (last_word == -1) continue;
			        for (c = 0; c < layer1_size; c++) neu1[c] += syn0[c + last_word * layer1_size];
			        cw++;
			      }
			      if (cw) {
			        for (c = 0; c < layer1_size; c++) neu1[c] /= cw;
			        if (hs) for (d = 0; d < vocab[word].codelen; d++) {
			          f = 0;
			          l2 = vocab[word].point[d] * layer1_size;
			          // Propagate hidden -> output
			          for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1[c + l2];
			          if (f <= -MAX_EXP) continue;
			          else if (f >= MAX_EXP) continue;
			          else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
			          // 'g' is the gradient multiplied by the learning rate
			          g = (1 - vocab[word].code[d] - f) * (*alpha_address);
			          // Propagate errors output -> hidden
			          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
			          // Learn weights hidden -> output
			          for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * neu1[c];
			        }
			        // NEGATIVE SAMPLING
			        if (negative > 0) for (d = 0; d < negative + 1; d++) {
			          if (d == 0) {
			            target = word;
			            label = 1;
			          } else {
			            next_random = next_random * (unsigned long long)25214903917 + 11;
			            target = table[(next_random >> 16) % table_size];
			            if (target == 0) target = next_random % (vocab_size - 1) + 1;
			            if (target == word) continue;
			            label = 0;
			          }
			          l2 = target * layer1_size;
			          f = 0;
			          for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1neg[c + l2];
			          if (f > MAX_EXP) g = (label - 1) * (*alpha_address);
			          else if (f < -MAX_EXP) g = (label - 0) * (*alpha_address);
			          else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * (*alpha_address);
			          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
			          for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * neu1[c];
			        }
			        // hidden -> in
			        for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
			          c = sentence_position - window + a;
			          if (c < 0) continue;
			          if (c >= sentence_length) continue;
			          last_word = sen[c];
			          if (last_word == -1) continue;
			          for (c = 0; c < layer1_size; c++) syn0[c + last_word * layer1_size] += neu1e[c];
			        }
			      }
			    } else {  //train skip-gram
			      for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
			        c = sentence_position - window + a;
			        if (c < 0) continue;
			        if (c >= sentence_length) continue;
			        last_word = sen[c];
			        if (last_word == -1) continue;
			        l1 = last_word * layer1_size;
			        for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
			        // HIERARCHICAL SOFTMAX
			        if (hs) for (d = 0; d < vocab[word].codelen; d++) {
			          f = 0;
			          l2 = vocab[word].point[d] * layer1_size;
			          // Propagate hidden -> output
			          for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1[c + l2];
			          if (f <= -MAX_EXP) continue;
			          else if (f >= MAX_EXP) continue;
			          else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
			          // 'g' is the gradient multiplied by the learning rate
			          g = (1 - vocab[word].code[d] - f) * (*alpha_address);
			          // Propagate errors output -> hidden
			          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
			          // Learn weights hidden -> output
			          for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * syn0[c + l1];
			        }
			        // NEGATIVE SAMPLING
			        if (negative > 0) for (d = 0; d < negative + 1; d++) {
			          if (d == 0) {
			            target = word;
			            label = 1;
			          } else {
			            next_random = next_random * (unsigned long long)25214903917 + 11;
			            target = table[(next_random >> 16) % table_size];
			            if (target == 0) target = next_random % (vocab_size - 1) + 1;
			            if (target == word) continue;
			            label = 0;
			          }
			          l2 = target * layer1_size;
			          f = 0;
			          for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
			          if (f > MAX_EXP) g = (label - 1) * (*alpha_address);
			          else if (f < -MAX_EXP) g = (label - 0) * (*alpha_address);
			          else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * (*alpha_address);
			          for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
			          for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];
			        }
			        // Learn weights input -> hidden
			        for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];
			      }
			    }
			    sentence_position++;
			    if (sentence_position >= sentence_length) {
			      sentence_length = 0;
			      continue;
			    }
			  }
			  fclose(fi);
			  free(neu1);
			  free(neu1e);
			  
			  //printf("nums: %lld\n", nums);
			  pthread_exit(NULL);
		};
		void Word::InitUnigramTable() {
			int a, i;
			  double train_words_pow = 0;
			  double d1, power = 0.75;
			  table = (int *)malloc(table_size * sizeof(int));
			  for (a = 0; a < vocab_size; a++) train_words_pow += pow(vocab[a].cn, power);
			  i = 0;
			  d1 = pow(vocab[i].cn, power) / train_words_pow;
			  for (a = 0; a < table_size; a++) {
			    table[a] = i;
			    if (a / (double)table_size > d1) {
			      i++;
			      d1 += pow(vocab[i].cn, power) / train_words_pow;
			    }
			    if (i >= vocab_size) i = vocab_size - 1;
			  }
		}
	void Word::TrainModel() {
		long a, b,c, d;
		FILE *fo;
		pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
		printf("Starting training using file %s\n", train_file);
		//starting_alpha = alpha;
		if(read_vocab_file[0] != 0) {
			ReadVocab();
		}else{
			LearnVocabFromTrainFile();
		}
		cout<<"save_vocab_file: "<< save_vocab_file <<endl;
		if(save_vocab_file[0] != 0) {
			SaveVocab();
		}
		if(output_file[0] == 0) {
			return;
		}
		
		InitNet();
		
		if(negative > 0) {
			InitUnigramTable();
		}
		float starting_alpha0 = alpha;
		float alpha0 = alpha;
		start = clock();
		  for (a = 0; a < num_threads; a++) {
		  	Args * args;
		  	if(NULL == (args = (Args *)malloc(sizeof(Args)))) {
		  		cout<<"create args error"<<endl;
		  		return ;
		  	}
		  	args->id = (void *) a;
		  	args->train_file = train_file;
			args->file_size = file_size;
			args->num_threads = num_threads;
			args->word_count_actual = &word_count_actual;
			args->debug_mode = debug_mode;
			args->alpha = &alpha0;
			args->starting_alpha = &starting_alpha0;
			args->train_words = train_words;
			args->sample = sample;
			args->vocab = &vocab;
			args->window = window;
			args->start = &start;
			args->cbow = cbow;
			args->negative = negative;
			args->syn1 = &syn1;
			args->hs = hs;
			args->syn0 = &syn0;
			args->syn1neg = &syn1neg;
			args->expTable = &expTable;
			args->table = &table;
			args->vocab_size = vocab_size; 
			args->vocab_hash = &vocab_hash;
			args->iter = iter;
			args->layer1_size = layer1_size;
			
		  	pthread_create(&pt[a], NULL, TrainModelThread, (void *)args);
		  } 
		  for (a = 0; a < num_threads; a++) {
		  	pthread_join(pt[a], NULL);
		  }
		  cout <<"alpha:"<<alpha0<<endl;
		  fo = fopen(output_file, "wb");

		  if (classes == 0) {
		    // Save the word vectors
		    fprintf(fo, "%lld %lld\n", vocab_size, layer1_size);
		    for (a = 0; a < vocab_size; a++) {
		      fprintf(fo, "%s ", vocab[a].word);
		      if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn0[a * layer1_size + b], sizeof(float), 1, fo);
		      else for (b = 0; b < layer1_size; b++) fprintf(fo, "%lf ", syn0[a * layer1_size + b]);
		      fprintf(fo, "\n");
		    }
		  } else {
		    // Run K-means on the word vectors
		    int clcn = classes, iter = 10, closeid;
		    int *centcn = (int *)malloc(classes * sizeof(int));
		    int *cl = (int *)calloc(vocab_size, sizeof(int));
		    float closev, x;
		    float *cent = (float *)calloc(classes * layer1_size, sizeof(float));
		    for (a = 0; a < vocab_size; a++) cl[a] = a % clcn;
		    for (a = 0; a < iter; a++) {
		      for (b = 0; b < clcn * layer1_size; b++) cent[b] = 0;
		      for (b = 0; b < clcn; b++) centcn[b] = 1;
		      for (c = 0; c < vocab_size; c++) {
		        for (d = 0; d < layer1_size; d++) cent[layer1_size * cl[c] + d] += syn0[c * layer1_size + d];
		        centcn[cl[c]]++;
		      }
		      for (b = 0; b < clcn; b++) {
		        closev = 0;
		        for (c = 0; c < layer1_size; c++) {
		          cent[layer1_size * b + c] /= centcn[b];
		          closev += cent[layer1_size * b + c] * cent[layer1_size * b + c];
		        }
		        closev = sqrt(closev);
		        for (c = 0; c < layer1_size; c++) cent[layer1_size * b + c] /= closev;
		      }
		      for (c = 0; c < vocab_size; c++) {
		        closev = -10;
		        closeid = 0;
		        for (d = 0; d < clcn; d++) {
		          x = 0;
		          for (b = 0; b < layer1_size; b++) x += cent[layer1_size * d + b] * syn0[c * layer1_size + b];
		          if (x > closev) {
		            closev = x;
		            closeid = d;
		          }
		        }
		        cl[c] = closeid;
		      }
		    }
		    // Save the K-means classes
		    for (a = 0; a < vocab_size; a++) fprintf(fo, "%s %d\n", vocab[a].word, cl[a]);
		    free(centcn);
		    free(cent);
		    free(cl);
		  }
		  fclose(fo);

	}
		void  Word::CreateBinaryTree() {
			 long long a, b, i, min1i, min2i, pos1, pos2, point[MAX_CODE_LENGTH];
			  char code[MAX_CODE_LENGTH];
			  long long *count = (long long *)calloc(vocab_size * 2 + 1, sizeof(long long));
			  long long *binary = (long long *)calloc(vocab_size * 2 + 1, sizeof(long long));
			  long long *parent_node = (long long *)calloc(vocab_size * 2 + 1, sizeof(long long));
			  for (a = 0; a < vocab_size; a++) count[a] = vocab[a].cn;
			  for (a = vocab_size; a < vocab_size * 2; a++) count[a] = 1e15;
			  pos1 = vocab_size - 1;
			  pos2 = vocab_size;
			  // Following algorithm constructs the Huffman tree by adding one node at a time
			  for (a = 0; a < vocab_size - 1; a++) {
			    // First, find two smallest nodes 'min1, min2'
			    if (pos1 >= 0) {
			      if (count[pos1] < count[pos2]) {
			        min1i = pos1;
			        pos1--;
			      } else {
			        min1i = pos2;
			        pos2++;
			      }
			    } else {
			      min1i = pos2;
			      pos2++;
			    }
			    if (pos1 >= 0) {
			      if (count[pos1] < count[pos2]) {
			        min2i = pos1;
			        pos1--;
			      } else {
			        min2i = pos2;
			        pos2++;
			      }
			    } else {
			      min2i = pos2;
			      pos2++;
			    }
			    count[vocab_size + a] = count[min1i] + count[min2i];
			    parent_node[min1i] = vocab_size + a;
			    parent_node[min2i] = vocab_size + a;
			    binary[min2i] = 1;
			  }
			  // Now assign binary code to each vocabulary word
			  for (a = 0; a < vocab_size; a++) {
			    b = a;
			    i = 0;
			    while (1) {
			      code[i] = binary[b];
			      point[i] = b;
			      i++;
			      b = parent_node[b];
			      if (b == vocab_size * 2 - 2) break;
			    }
			    vocab[a].codelen = i;
			    vocab[a].point[0] = vocab_size - 2;
			    for (b = 0; b < i; b++) {
			      vocab[a].code[i - b - 1] = code[b];
			      vocab[a].point[i - b] = point[b] - vocab_size;
			    }
			  }
			  free(count);
			  free(binary);
			  free(parent_node);
		}
	void Word::InitNet() {
		long long a, b;
		  unsigned long long next_random = 1;
		  a = posix_memalign((void **)&syn0, 128, (long long)vocab_size * layer1_size * sizeof(float));
		  if (syn0 == NULL) {printf("Memory allocation failed\n"); exit(1);}
		  if (hs) {
		    a = posix_memalign((void **)&syn1, 128, (long long)vocab_size * layer1_size * sizeof(float));
		    if (syn1 == NULL) {printf("Memory allocation failed\n"); exit(1);}
		    for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++)
		     syn1[a * layer1_size + b] = 0;
		  }
		  if (negative>0) {
		    a = posix_memalign((void **)&syn1neg, 128, (long long)vocab_size * layer1_size * sizeof(float));
		    if (syn1neg == NULL) {printf("Memory allocation failed\n"); exit(1);}
		    for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++)
		     syn1neg[a * layer1_size + b] = 0;
		  }
		  for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++) {
		    next_random = next_random * (unsigned long long)25214903917 + 11;
		    syn0[a * layer1_size + b] = (((next_random & 0xFFFF) / (float)65536) - 0.5) / layer1_size;
		  }
		  CreateBinaryTree();
	}
	/**
		get word and word 
	*/
	const long long max_size = 2000;         // max length of strings
	const long long N = 40;                  // number of closest words that will be shown
	const long long max_w = 50; 			// max length of vocabulary entries
	int Word::Get(char* st1,Isolate* isolate, Handle<Object> result) {
		//char st1[max_size];
		//strcpy(st1, word);
		char st[100][max_size];
		long long words, size, a, b, c, d, cn, bi[100];
		float dist, len , bestd[N], vec[max_size];
		float* M;
		char *bestw[N];
		FILE *file = fopen(output_file, "rb");
		if(file == NULL) {
			cout<<output_file << "have error"<<endl;
			return 0;
		}
		fscanf(file, "%lld", &words);
  		fscanf(file, "%lld", &size);
		char* vocabData;
		vocabData = (char *)malloc((long long)words * max_w * sizeof(char));
		 for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
		  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
		  if (M == NULL) {
		    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)words * size * sizeof(float) / 1048576, words, size);
		    return -1;
		  }
		for (b = 0; b < words; b++) {
		    a = 0;
		    while (1) {
		       vocabData[b * max_w + a] = fgetc(file);
		      if (feof(file) || ( vocabData[b * max_w + a] == ' ')) break;
		      if ((a < max_w) && (vocabData[b * max_w + a] != '\n')) a++;
		    }
		    vocabData[b * max_w + a] = 0;
		    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, file);
		    len = 0;
		    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
		    len = sqrt(len);
		    for (a = 0; a < size; a++) M[a + b * size] /= len;
		  }
		  fclose(file);

		  for (a = 0; a < N; a++) bestd[a] = 0;
			for (a = 0; a < N; a++) bestw[a][0] = 0;
				 a = 0;
		    
		    cn = 0;
		    b = 0;
		    c = 0;
		    while (1) {
		      st[cn][b] = st1[c];
		      b++;
		      c++;
		      st[cn][b] = 0;
		      if (st1[c] == 0) break;
		      if (st1[c] == ' ') {
		        cn++;
		        b = 0;
		        c++;
		      }
		    }
		   
		    cn++;
		    for (a = 0; a < cn; a++) {
		      for (b = 0; b < words; b++) if (!strcmp(&vocabData[b * max_w], st[a])) break;
		      if (b == words) b = -1;
		      bi[a] = b;
		      printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
		      if (b == -1) {
		        printf("Out of dictionary word!\n");
		        break;
		      }
		    }
		    if (b == -1) return 0;
		    printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
		    for (a = 0; a < size; a++) vec[a] = 0;
		    for (b = 0; b < cn; b++) {
		      if (bi[b] == -1) continue;
		      for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
		    }
		    len = 0;
		    for (a = 0; a < size; a++) len += vec[a] * vec[a];
		    len = sqrt(len);
		    for (a = 0; a < size; a++) vec[a] /= len;
		    for (a = 0; a < N; a++) bestd[a] = -1;
		    for (a = 0; a < N; a++) bestw[a][0] = 0;
		    for (c = 0; c < words; c++) {
		      a = 0;
		      for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
		      if (a == 1) continue;
		      dist = 0;
		      for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
		      for (a = 0; a < N; a++) {
		        if (dist > bestd[a]) {
		          for (d = N - 1; d > a; d--) {
		            bestd[d] = bestd[d - 1];
		            strcpy(bestw[d], bestw[d - 1]);
		          }
		          bestd[a] = dist;
		          strcpy(bestw[a], &vocabData[c * max_w]);
		          break;
		        }
		      }
		    }
		    //for (a = 0; a < N; a++) printf("%50s\t\t%f\n", bestw[a], bestd[a]);
		    for(a = 0; a < N; a++) {
				Handle<Value> key = String::NewFromUtf8(isolate, bestw[a]);
				Handle<Value> value = NumberObject::New(isolate, bestd[a]); 
				result->Set(key, value);
			}
/**
///////////////////////////
		while(1) {

			cn = 0;
			b = 0;
			c = 0;
			st[cn][b] = 0;
			if(word[c] == 0) {
				break;
			} 
			if(word[c] ==  ' ') {
				cn++;
				b = 0;
				c++;
			}

		}
		cn++;
		for( a=0; a < cn; a++) {
			for(b = 0; b < words; b++) {
				if(!strcmp(&vocabData[b * max_w], st[a])) {
					return 0;
				}
				if(b == words) {
					b = -1;
				}
				bi[a] = b;
				printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
				if (b == -1) {
			        printf("Out of dictionary word!\n");
			        return 0;
			      }
			}
		}
		if(b == -1) { return 0; }
		for(a = 0; a <size; a++) {
			vec[a] = 0;
		}
		for(b = 0; b < cn; b++) {
			if(bi[b] == -1) {
				 continue;
			}
			for(a = 0; a < size; a++) {
				vec[a] += M[a+bi[b] * size];
			}

		}
		len = 0;
			for(a = 0; a < size; a++) {
				len += vec[a] * vec[a];
			}
			len = sqrt(len);
			for(a = 0; a < size; a++) {
				vec[a] /= len;
			}
			for(a = 0; a < size; a++) {
				len += vec[a] * vec[a];
			}
			len = sqrt(len);
			for(a = 0; a < size; a++) {
				vec[a] /= len;
			}
			for(a = 0; a < N ; a++) {
				bestd[a] = -1;
				bestw[a][0] = 0;
			}
			for(c = 0; c < words; c++) {
				a = 0;
				for(b = 0; b < cn; b++) {
					if(bi[b] == c) {
						a = 1;
					}
				}
				if(a == 1) {
					continue;
				}
				dist = 0;
				for(a = 0; a < size; a++) {
					dist += vec[a] * M[a+ c *size];
				}
				for(a = 0; a < N; a++) {
					if(dist > bestd[a]) {
						for(d = N -1; d > a; d--) {
							bestd[d] = bestd[d -1];
							strcpy(bestw[d], bestw[d - 1]);
						}
						bestd[a] = dist;
						strcpy(bestw[a], &vocabData[c * max_w]);
						break;
					}
				}
			}
			for(a = 0; a < N; a++) {
				Handle<Value> key = String::NewFromUtf8(isolate, bestw[a]);
				Handle<Value> value = NumberObject::New(isolate, bestd[a]); 
				result->Set(key, value);
			}
		
		*/
		return 1;
	}


	void Word::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		//Word = function() {}
		tpl->SetClassName(String::NewFromUtf8(isolate, "Word"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		//Word.prototype.add= AddWord;
		NODE_SET_PROTOTYPE_METHOD(tpl, "add", AddWord);
		NODE_SET_PROTOTYPE_METHOD(tpl, "get", Get);
		constructor.Reset(isolate, tpl->GetFunction());
		//module.word = Word;
		exports->Set(String::NewFromUtf8(isolate, "Word"),
				tpl->GetFunction());
	}
	void Word::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if(args.IsConstructCall()) {
			//Handle<Object> object = Handle<Object>::Cast(args[0]);
			Word* word = new Word(args);
			word->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}else{
			const int argc =  1;
			Local<Value> argv[argc] = {args[0]};
			Local<Context> context = isolate->GetCurrentContext();
			Local<Function> cons = Local<Function>::New(isolate, constructor);
			Local<Object> result = 
				cons->NewInstance(context, argc, argv).ToLocalChecked();
			args.GetReturnValue().Set(result);
		}
	}
	void Word::AddWord(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		Word* word = ObjectWrap::Unwrap<Word>(args.Holder());
	 	//Handle<Value> fieldValue = word->config_->Get(String::NewFromUtf8(isolate, "file"));
	  	//String::Utf8Value ascii2(fieldValue);
	  	//printf("%s\n", *ascii2);
	  	cout<<"addWord:"<< (word->layer1_size) <<endl;
	  	args.GetReturnValue().Set(Number::New(isolate, 0));
	}
	void Word::Get(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		Word* worder = ObjectWrap::Unwrap<Word>(args.Holder());
		Handle<Object> wordValue = Handle<Object>::Cast(args[0]);
		String::Utf8Value word(wordValue);
		Handle<Object> result = Object::New(isolate);
		 int r = worder->Get(*word, isolate,result);
	 	if(r == 1) {
	 		cout<<"????"<<endl;
		 	Local<Object> lresult = Local<Object>::New(isolate, result);
			args.GetReturnValue().Set(lresult);
		}
		 
	}
	void Word::NewInstance(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		const unsigned argc = 1;
		Local<Value> argv[argc] = {args[0]};
		Local<Function> cons = Local<Function>::New(isolate, constructor);
		Local<Context> context = isolate->GetCurrentContext();
		Local<Object> instance = 
			cons->NewInstance(context, argc, argv).ToLocalChecked();
		args.GetReturnValue().Set(instance);
	}
}