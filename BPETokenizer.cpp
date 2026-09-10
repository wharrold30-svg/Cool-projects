#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <limits>

class BPETokenizer{
  private:
	std::unordered_map<int, std::string> m_idToToken;
	std::unordered_map<std::string, int> m_tokenToId;
	std::map<std::pair<int, int>, int> m_merges; //maps (tokenA, tokenB)-> newToken

	void initVocab(const std::string &corpus){
		m_idToToken.clear();
		m_tokenToId.clear();
		m_merges.clear();

		for (unsigned char ch : corpus){
			std::string s(1, ch);
			if (m_tokenToId.find(s) == m_tokenToId.end()){
				int newId = static_cast<int>(m_tokenToId.size());
				m_tokenToId[s] = newId;
				m_idToToken[newId] = s;
			}
		}
	}

  public:
	//will use = default jn production but for debug purposes we are printing out constructed
	BPETokenizer(){
		std::puts("Tokenizer constructed");
	}

	~BPETokenizer(){
		std::puts("Tokenizer destroyed");
	}

	void train(const std::string &corpus, size_t targetVocabSize){
		initVocab(corpus);
		//convert inital text to initial byte token IDs
		std::vector<int> tokens;
		tokens.reserve(corpus.size());
		for (unsigned char ch : corpus){
			tokens.push_back(m_tokenToId[std::string(1, ch)]);
		}

		while (m_tokenToId.size() < targetVocabSize){
			//Count frequency of adjacent token pairs
			std::map<std::pair<int, int>, int> pairCounts;
			for (size_t i = 0; i + 1 < tokens.size(); ++i){
				pairCounts[{tokens[i], tokens[i + 1]}]++;
			}
			//No pairs
			if (pairCounts.empty())  break;

			//Find the most frequent pairs
			std::pair<int, int> bestPair;
			int maxFreq = 0;
			for (const auto &[pair, count] : pairCounts){
				if (count > maxFreq){
					maxFreq = count;
					bestPair = pair;
				}
			}
			//Stop if no pairs repeat
			if (maxFreq < 2)  break;

			//Create new token
			int newId = static_cast<int>(m_tokenToId.size());
			std::string mergedStr{m_idToToken[bestPair.first] + m_idToToken[bestPair.second]};
			m_tokenToId[mergedStr] = newId;
			m_idToToken[newId] = mergedStr;
			m_merges[bestPair] = newId;

			//Replace all occurrences of bestPair
			//with newId in training tokens
			std::vector<int> newTokens;
			newTokens.reserve(tokens.size());
			for (size_t i = 0; i < tokens.size(); ++i){
				if (i + 1 < tokens.size() &&
					tokens[i] == bestPair.first &&
					tokens[i + 1] == bestPair.second){
					newTokens.push_back(newId);
					++i;
				}
				else{
					newTokens.push_back(tokens[i]);
				}
			}
			tokens = std::move(newTokens);
		}
	}

	//Encode text into integer token IDs
	std::vector<int> encode(const std::string &text) const{
		std::vector<int> tokens;
		for (unsigned char ch : text){
			std::string s(1, ch);
			auto it = m_tokenToId.find(s);
			if (it != m_tokenToId.end()){
				tokens.push_back(it->second);
			}
			//I guess this is where I'll handle new tokes????
		}

		//Appy learned merges iteratively
		while (tokens.size() > 2){
			int minRank = std::numeric_limits<int>::max();
			std::pair<int, int> bestPair;
			size_t mergeIdx = 0;
			bool foundMerge = false;

			//Find thepair with the earlier merge priority
			for (size_t i = 0; i + 1 < tokens.size(); ++i){
				std::pair<int, int> pair = {tokens[i], tokens[i + 1]};
				auto it = m_merges.find(pair);
				if (it != m_merges.end()){
					if (it->second < minRank){
						minRank = it->second;
						bestPair = pair;
						mergeIdx = i;
						foundMerge = true;
					}
				}
			}
			//No more eligible merges left
			if (!foundMerge)   break;

			//Merge the best pair found
			std::vector<int> newTokens;
			newTokens.reserve(tokens.size());
			for (size_t i = 0; i < tokens.size(); ++i){
				if (i + 1 < tokens.size() &&
					tokens[i] == bestPair.first &&
					tokens[i + 1] == bestPair.second){
					newTokens.push_back(minRank);
					++i;
				}
				else{
					newTokens.push_back(tokens[i]);
				}
			}
			tokens = std::move(newTokens);
		}
		return tokens;
	}

	//Decide integer token IDs back ti a text string
	std::string decode(const std::vector<int> &tokens) const{
		std::string results;
		for (int id : tokens){
			auto it = m_idToToken.find(id);
			if (it != m_idToToken.end()){
				results += it->second;
			}
		}
		return results;
	}

}; //BPETokenizer

int main(int argc, char *argv[]){
	std::string corpus{"low lower newest newest lowest low low low"};
	std::puts("Testing default constructor");
	BPETokenizer tokenizer;

	std::puts("Testing tokenizer trainning");
	//Train up to a vocabulary size of 20
	tokenizer.train(corpus, 20);

	std::puts("\nTesting tokenizer encode");
	std::string input{"lowest lower"};
	std::vector<int> encoded = tokenizer.encode(input);
	std::cout << "\nEncoded IDs:  [";
	for (int id : encoded){
		std::cout << id << " ";
	}
	std::cout << "]\n";

	std::puts("\nTesting tokenizer encode");
	std::string decoded = tokenizer.decode(encoded);
	std::cout << "Original Text:  \"" << input << "\"\n";
	std::cout << "Decoded Text:  \"" << decoded << "\"\n";

	std::cout << input.size() << " bytes in original text\n";
	std::cout << encoded.size() << " tokens in encoded\n";
	std::cout << "Compression ratio ==  " << input.size() / encoded.size() << "\n";
	return 0;
}

