var latte_word = require("../build/Release/latte_word.node");
/**
train text8 
-output vectors.bin 
-cbow 1 
-size 200 
-window 8 
-negative 25 
-hs 0 
-sample 1e-4
 -threads 20 
 -binary 1 
 -iter 15
*/
var word = new latte_word.Word({
	output: "./text8.bin",
	size: 200,
	train: "./text8_0",
	"save-vocab": "sVocab",
	debug: 2,
	binary: 1,
	cbow: 1,
	//alpha: 0.01,
	window: 8,
	sample: 0.0001,
	hs: 0,
	negative: 25,
	threads: 20,
	iter: 1000,
	"min-count": 5,
	classes: 0
});
var Fs = require("fs");
var data = Fs.readFileSync("text8_0").toString();
word.add(data);
var result = word.get("a");
for(var i in result) {
	console.log(i, +result[i]);
}