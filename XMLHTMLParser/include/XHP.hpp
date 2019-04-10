#pragma once
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <streambuf>
#include <functional> 
#include <cctype>
#include <locale>
#include <unordered_map>
#include <regex>
#include<iostream>
#include <iomanip>
using namespace std;
namespace XHP {


	template<typename Out>
	void split(const string &s, char delim, Out result) {
		stringstream ss(s);
		string item;
		while (getline(ss, item, delim)) {
			*(result++) = item;
		}
	}
	vector<string> split(const string &s, char delim) {
		vector<string> elems;
		split(s, delim, back_inserter(elems));
		return elems;
	}
	vector<string> splitIgnore(const string s, char delim, char bw) {
		int start = 0;
		bool ignore = true;
		vector<string> elems;
		for (int x = 0; x < s.size(); x++) {
			if (s[x] == delim && ignore) {
				elems.push_back(s.substr(start, x - start));
				start = x + 1;
			}
			if (s[x] == bw)ignore = !ignore;
		}
		elems.push_back(s.substr(start, s.size() - start));

		return(elems);
	}
	void removeCharsFromString(string &str, string charsToRemove) {
		for (unsigned int i = 0; i < charsToRemove.length(); ++i) {
			str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
		}
	}
	string substring(string &str, int begin, int ending) {
		return(str.substr(begin, ending - begin));
	}
	string trim(const string& str)
	{
		size_t first = str.find_first_not_of(" \n\t");
		if (string::npos == first)
		{
			return str;
		}
		size_t last = str.find_last_not_of(" \n\t");
		return str.substr(first, (last - first + 1));
	}
	inline string strMult(string str, int times) {
		string out;
		for (int i = 0; i < times; i++) {
			out += str;
		}

		return(out);
	}
	inline string getBetween(string str, char sourroundings) {
		auto threeparts = split(str, sourroundings);
		if (threeparts.size() == 3) {
			return(threeparts[1]);
		}
		else {
			return(str);
		}
	}
	class Node {
	public:

		string tag;
		map<string, string> attributes;
		vector<Node*> children;
		string innertext="";
		string textleft;
		string textright;
		bool closed = false;
		Node() {

		}
		Node(string tagname) {
			tag = tagname;
		}
		inline vector<Node*> getByTag(string tag) {
			vector<Node*> out;
			for (Node* n : children) {
				if (n->tag == tag) {
					out.push_back(n);
				}
				vector<Node*> res = n->getByTag(tag);
				out.insert(out.end(), res.begin(), res.end());
			}
			return(out);
		}
		inline void parseAttributes(string rawinnertag) {
			vector<string> splitted = splitIgnore(rawinnertag, ' ', '"');
			int i = 0;
			for (auto attr : splitted) {
				if (i > 0) {
					if (!attr.empty()) {
						vector<string> eqsplit = splitIgnore(trim(attr), '=', '"');
						if (eqsplit.size() == 2) {
							if (eqsplit[1][0] == '"'&&eqsplit[1][eqsplit[1].length() - 1] == '"') {
								eqsplit[1] = eqsplit[1].substr(1, eqsplit[1].length() - 2);
							}
							attributes[eqsplit[0]] = eqsplit[1];
							//printf("%s = %s \n", eqsplit[0].c_str(), attributes[eqsplit[0]].c_str());
						}
						if (eqsplit.size() == 1) {
							attributes[eqsplit[0]] = "";
							//printf("%s = \"\" \n", eqsplit[0].c_str());
						}

					}
				}
				i++;
			}
		}
		inline string text() {
			string out("");
			if(!textleft.empty())out += textleft;
			if(!innertext.empty())out += innertext;
			if (!textright.empty())out += textleft;
			for (auto c : children) {
				out += c->text();
			}
			return(out);
		}
		inline bool isWhiteSpace(char c){
			return c==' '||c=='\n'|| c == '\t' || c == '\f' || c == '\r';
		}
		inline bool matchesselector(string selector) {

			if (selector[0] == '*')return true;
			if (selector == tag)return true;
			if (attributes.count("class")) {
				auto splitted = split(selector, '.');
				string classname = attributes["class"];
				if (splitted.size() == 2) {
					if (splitted[1].compare(classname) == 0) {
						if (splitted[0].empty() || splitted[0] == tag) {
							return(true);
						}
					}

				}
			}
			if (attributes.count("id")) {
				auto splitted = split(selector, '#');
				string id = attributes["id"];
				if (splitted.size() == 2) {
					if (splitted[1] == id) {
						if (splitted[0].empty() || splitted[0] == tag) {
							return(true);
						}
					}

				}
			}
			int brstart=selector.find('[');
			int brend=selector.find(']');
			if(brstart!=-1&&brend!=-1){
				auto between=split(selector.substr(brstart+1, brend-brstart-1),'=');
				string tagname=selector.substr(0, brstart);
				if(tagname.empty()||tagname==tag){
					if(between.size()==1){
						if(attributes.count(between[0])){
							return true;
						}
					}else if(between.size()==2){
						if(attributes.count(between[0])){
							if(attributes[between[0]]==between[1]){
								return true;
							}
						}
					}
				}

			}

			/*while(!selector.empty()){
				const char c=selector[0];
				if(isWhiteSpace(c)){

				}
			}*/
			return(false);
		}
	};
	class NodeModel {
	public:

		string tag;
		int count = 1;
		unordered_map<string, map<string, int>> attributes;
		unordered_map<string, NodeModel*> children;
		string text;
		string* textleft;
		string* textright;
		bool closed = false;
		NodeModel() {

		}
		NodeModel(string tagname) {
			tag = tagname;
		}
		inline void addChild(Node* n) {
			if (children.count(n->tag)) {

				children[n->tag]->count++;
			}
			else {
				children[n->tag] = new NodeModel(n->tag);
			}
			for (Node* c : n->children) {
				children[n->tag]->addChild(c);
			}

		}
		inline void print(int tab) {
			printf("%s %s\t(%i)\n", strMult("-", tab).c_str(), tag.c_str(), count);
			for (auto nm : children) {
				nm.second->print(tab + 1);
			}
		}
		inline void print() {
			print(0);
		}
		/*inline vector<Node*> getByTag(string tag) {
		vector<Node*> out;
		for (Node* n : children) {
		if (n->tag == tag) {
		out.push_back(n);
		}
		vector<Node*> res = n->getByTag(tag);
		out.insert(out.end(), res.begin(), res.end());
		}
		return(out);
		}*/

	};
	typedef void(*NodeCallback)(Node*, void* data);
	typedef struct {
		NodeCallback ncb;
		void* data;
	}NodeCallbackData;
	class Document {
	private:

		const char c_ts = '<';//tag start
		const char c_te = '>';//tag end
		const char c_ad = '=';//attribute delimiter
		const char c_as = ' ';//attribute seperator
		const char c_em = '/';//tag end marker
		const char c_ac = '"';//attribute
		inline bool isCharIn(string str, char in) {
			for (char c : str) {
				if (c == in)return(true);
			}
			return(false);
		}
		inline bool isSpace(char in) {
			return(isCharIn(" \t\n", in));
		}

		inline void printTree(int tabs, Node* n) {
			//printf("%s\n",  n->text.c_str());
			if (n->children.empty()) {
				printf("%s-%s\n", strMult("-", tabs).c_str(), n->tag.c_str());
			}
			else {
				printf("%s+%s\n", strMult("-", tabs).c_str(), n->tag.c_str());
				for (Node* c : n->children) {
					printTree(tabs + 1, c);
				}
			}

		}
	public:
		ofstream of;
		vector<Node*> allnodes;
		vector<Node*> nodetree;
		unsigned int tagstart = 0;
		unsigned int tagend = 0;
		unsigned int lasttagend = 0;
		int tabi = 0;
		bool opentag = false;
		bool insidescript = false;
		string rawhtml;
		Node* rootnode = new Node("");
		inline void clear(){
			rawhtml="";
			tagstart = 0;
			tagend = 0;
			lasttagend = 0;
			tabi = 0;
			opentag = false;
			insidescript = false;
			rootnode = new Node("");
			allnodes.clear();
			nodetree.clear();
		}
		inline void write(string data) {
			int i = rawhtml.size() - 1;
			rawhtml += data;
			//printf("%s", data.c_str(), data.length());
			for (char c : data) {

				if (c == c_ts) {
					tagstart = i;
					opentag = true;

				}
				else
					if (c == c_te) {
						tagend = i;
						if (opentag) {
							string tag = rawhtml.substr(tagstart + 2, tagend - tagstart - 1);

							int firstspace = tag.find(" ");
							if (firstspace == -1)firstspace = tag.length();
							string tagname = tag.substr(0, firstspace);
							//printf("b:%s\nl:%s\n", tag.c_str(), tagname.c_str());
							if (insidescript) {
								//printf("%s\n", tagname.c_str());
								if (tagname.compare("/script") == 0) {
									insidescript = false;
									//printf("-%s\n", tag.c_str());
									tabi--;
								}
							}
							else {
								if (tagname.compare("script") == 0) {
									insidescript = true;
									Node* n = new Node(tagname);
									n->parseAttributes(tag);
									allnodes.push_back(n);
									nodetree.push_back(n);

									//printf("+%s\n", tagname.c_str());
								}
								else {
									if (tag[0] == c_em) {
										tagname = tagname.substr(1, tagname.size() - 1);
										//printf("-%s\n", tagname.c_str());
										for (long x = nodetree.size() - 1; x > 0; x--)
										{

											if (nodetree[x]->tag.compare(tagname) == 0 && nodetree[x]->closed == false) {
												if (x == nodetree.size() - 1) {
													string text = rawhtml.substr(lasttagend + 2, tagstart - lasttagend - 1);
													//printf("text inner:\"%s\"\n", text.c_str());
													nodetree[x]->innertext = text;
												}
												else {
													string text = rawhtml.substr(lasttagend + 2, tagstart - lasttagend - 1);
													//printf("text right:\"%s\"\n", text.c_str());
													nodetree[x]->textright = text;
												}
												for (size_t y = nodetree.size() - 1; y > x; y--)
												{
													nodetree[x]->children.insert(nodetree[x]->children.begin(), nodetree.back());

													nodetree.pop_back();
												}
												nodetree[x]->closed = true;
												for (auto cb : callbacks) {
													if (nodetree[x]->matchesselector(cb.first)) {
														cb.second.ncb(nodetree[x], cb.second.data);
													}
												}
												x = 0;

											}
											
										}
										tabi--;
										
									}
									else if (tag[0] == L'!') {
										Node* n = new Node("!");
										n->innertext = tag;

										allnodes.push_back(n);
										nodetree.push_back(n);
									}
									else {
										Node* n = new Node(tagname);
										n->parseAttributes(tag);
										for (auto cb : callbacks) {
											if (n->matchesselector(cb.first)) {
												cb.second.ncb(n, cb.second.data);
											}
										}
										allnodes.push_back(n);
										nodetree.push_back(n);
										//printf("+%s\n", tagname.c_str());
										tabi++;
										string text = rawhtml.substr(lasttagend + 2, tagstart - lasttagend - 1);
										n->textleft = text;
										//printf("text left:\"%s\"\n", text.c_str());
									}
								}
								opentag = false;
							}
							lasttagend = tagend;
						}

					}

				i++;
			}
			
			//printf("tabi:%i\n", tabi);
		}
		inline Node* getRootNode() {
			if(rootnode->children.empty())
			for (auto n : nodetree) {
				rootnode->children.push_back(n);
			}
			return(rootnode);
		}
		inline Document() {
		}
		inline Document(string data) {
			this->write(data);
			//printf("%s", data.c_str());
			/*int to=0;
			int tc=0;
			int ste = -1;//start tag end
			for (int i = 0; i < data.size(); i++) {
				if (data[i] == ts) {
					to = i;
				}
				if (data[i] == te) {

					tc = i;
					string tag=data.substr(to + 1, tc - to );
					if (tag[tag.size() - 1] == em) {
						tag = tag.substr(0, tag.size() - 1);
					}
					if (tag[tag.size() - 1] == te) {
						tag = tag.substr(0, tag.size() - 1);
					}
					printf("+'%s'\n", tag.c_str());
					vector<string> args = splitIgnore(tag, L' ', L'"');
					string tagname = args[0];

					removeCharsFromString(tag, "\n\t");
					if (tag[0] == em) {
						//int so=-1;
						//int se=-1;
						tag = tagname.substr(1, tagname.size() - 1);
						int y =nodetree.size();
						Node* parent=NULL;
						vector<Node*> children;
						int endindex=-1;
						while (y > 0) {
							endindex = y - 1;
							//printf("%s==%s\\n\n", nodetree[endindex]->tag.c_str(), tagname.c_str());

							if (nodetree[endindex]->tag == tagname) {
								parent= nodetree[endindex];
								y = 0;

							}
							else {
								children.push_back(nodetree[endindex]);

							}
							y--;
						}
						if (parent != NULL) {
							for (y = children.size() ; y > 0; y--) {
								parent->children.push_back(children[y-1]);
								nodetree.pop_back();

							}

						}
						else {
							printf("fail at%s\n", tag.c_str());
						}
						if(children.empty()) {
							if (ste != -1) {
								parent->text = data.substr(ste+1, to - ste-1);
							}
						}
						else {
							y = to - 1;
							string text;
							int tagi=0;
							while (y >= 0) {
								if (data[y] == ts)tagi++;
								if(y>0)if (data[y-1] == te)tagi++;

								if(tagi==0)
								text = data[y]+text;
								y--;
							}
							if(parent!=NULL)
							parent->text = text;
						}

					}
					else {

						int c = 0;
						ste = i;

						Node* nodenow = new Node(tagname);
						if (args.size() > 1) {
							for (int x = 1; x < args.size(); x++) {
								vector<string> attribute = split(args[x], '=');
								if (attribute.size() == 2) {
									string value = substring(attribute[1], attribute[1].find('"') + 1, attribute[1].rfind('"'));
									nodenow->attributes[attribute[0].c_str()]=value;
									//printf("\t'%s'='%s'\n", attribute[0].c_str(),value.c_str());

								}
								else if (attribute.size() == 1) {
									if (attribute[0] != " ") {
										//printf("\t'%s'\n", attribute[0].c_str());
										nodenow->attributes[attribute[0].c_str()] = "";
									}
								}
							}
						}
						allnodes.push_back(nodenow);
						nodetree.push_back(nodenow);
						printf("~'%s'\n", tagname.c_str());

					}

				}

			}*/
		};
		inline vector<Node*> getByTag(string tag) {
			return(getRootNode()->getByTag(tag));
		}
		inline void print() {
			for (Node* n : nodetree) {
				printTree(0, n);
			}
		}

		map<string, NodeCallbackData> callbacks;
		/*
		 * Lambda: [](XHP::Node* n, void* data) {code}
		 * Func: void bla(XHP::Node* n, void* data) {code}
		 */
		inline void addCallback(string selector, NodeCallback callback, void* data) {
			callbacks[selector] = { callback,data };
		}
		inline NodeModel* getNodeModel(Node* inn) {
			NodeModel* out = new NodeModel();
			for (Node* n : inn->children) {
				out->addChild(n);
			}
		}
		inline void printModel() {
			NodeModel* root = new NodeModel("root");
			for (Node* n : nodetree) {
				root->addChild(n);
			}
			root->print();
		}
	};
	string url_encode(const string &value) {
	    ostringstream escaped;
	    escaped.fill('0');
	    escaped << hex;

	    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
	        string::value_type c = (*i);

	        // Keep alphanumeric and other accepted characters intact
	        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
	            escaped << c;
	            continue;
	        }

	        // Any other characters are percent-encoded
	        escaped << uppercase;
	        escaped << '%' << setw(2) << int((unsigned char) c);
	        escaped << nouppercase;
	    }

	    return escaped.str();
	}
//#ifdef CURL_H
#include<curl/curl.h>
	size_t writeXHP(char *ptr, size_t size, size_t nmemb, void *userdata) {
		size_t realsize = size * nmemb;
		XHP::Document* doc = static_cast<XHP::Document*>(userdata);
		string str(ptr, realsize);
		//printf("%s",str.c_str());
		doc->write(str);


		return(realsize);
	}
	size_t curlwriteString(char *ptr, size_t size, size_t nmemb, void *userdata) {
		size_t realsize = size * nmemb;
		string* str = static_cast<string*>(userdata);
		
		str->append(ptr, realsize);


		return(realsize);
	}
	size_t writeFile(char *ptr, size_t size, size_t nmemb, void *userdata) {
		size_t realsize = size * nmemb;
		ofstream* str = static_cast<ofstream*>(userdata);
		str->write(ptr, realsize);
		



		return(realsize);
	}
	struct Multiple {
		ofstream* fileout;
		string* strout;
		Document* docout;
	};
	struct OldPage{
		string* olddata;
		string* oldheader;
		string* newheader;
		string* newdata;
		Document* docout;
		ofstream* fileout;
	};
	struct Page{
		string header;
		string download;
	};
	size_t writeMultiple(char *ptr, size_t size, size_t nmemb, void *userdata) {
		Multiple* mul = static_cast<Multiple*>(userdata);
		if (mul->docout != NULL) {
			writeXHP(ptr, size, nmemb, mul->docout);
		}
		if (mul->strout != NULL) {
			curlwriteString(ptr, size, nmemb, mul->strout);
		}
		if (mul->fileout != NULL) {
			writeFile(ptr, size, nmemb, mul->fileout);
		}
		size_t realsize = size * nmemb;
		return(realsize);
	}
	size_t writeCookies(char *ptr, size_t size, size_t nmemb, void *userdata) {
		size_t realsize = size * nmemb;
		map<string, string>* cookies = static_cast<map<string, string>*>(userdata);
		string str(ptr, realsize);
		//printf("%s header\n", str.c_str());
		if (str.find("Set-Cookie:") == 0) {
			//
			str = str.substr(string("Set-Cookie:").length());
			auto splitted = split(str, ';');
			for (auto elm : splitted) {
				auto eqsplit = split(elm, '=');
				if (eqsplit.size() == 2) {
					
					cookies->operator[](trim(eqsplit[0])) = trim(eqsplit[1]);
					//printf(" %s = %s \n", trim(eqsplit[0]).c_str(), trim(eqsplit[1]).c_str());
				}
			}
		}
		


		return(realsize);
	}
	size_t writeCompareHeader(char *ptr, size_t size, size_t nmemb, void *userdata) {
		size_t realsize = size * nmemb;
		OldPage* op = static_cast<OldPage*>(userdata);
		curlwriteString(ptr, size, nmemb, op->newheader);
		int cmp=op->oldheader->substr(0, op->newheader->size()).compare(op->newheader->c_str());
		if(cmp!=0){
			//printf("differs\n%s\n",op->newheader->c_str());
		}else{
			//printf("normal\n%s\n",op->newheader->c_str());
		}
		return(realsize);
	}
	class CurlCache {
	private:
		CURL* handle;
		map<string, string> cookies;

	public:
		map<string, string> cache;
		inline CurlCache() {
			
			
			
		}
		inline void download(const char* url) {
			handle = curl_easy_init();
			curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
			string newcache = "";
			curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlwriteString);
			curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&newcache);

			curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION,writeCookies);
			curl_easy_setopt(handle, CURLOPT_HEADERDATA,&cookies);

			curl_easy_setopt(handle, CURLOPT_COOKIE, getCookies().c_str());

			curl_easy_setopt(handle, CURLOPT_URL, url);
			curl_easy_perform(handle);
			cache[string(url)] = newcache;
			curl_easy_cleanup(handle);
		}
		inline void download(const char* url,const char* file) {
			download(url, file, NULL,false);
		}
		inline void download(const char* url, const char* file,Document* doc,bool redownload) {
			string urlstr = string(url);
			if (cache.count(urlstr)&&!redownload) {
				printf("loaded from cache\n");
				doc->write(cache[urlstr]);
				return;
			}
			handle = curl_easy_init();
			//curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
			//curl_easy_setopt(handle, CURLOPT_SSLENGINE, "dynamic");
			//curl_easy_setopt(handle, CURLOPT_SSLENGINE_DEFAULT, 1L);
			//curl_easy_setopt(handle, CURLOPT_SSLCERTTYPE,"PEM");
			//curl_easy_setopt(handle, CURLOPT_SSLCERT, "cacert.pem");
			string newcache="";
			ofstream* outfilestream=NULL;
			if (file != NULL) {
				outfilestream = new ofstream(file);
			}
			Multiple m = { outfilestream,&newcache,doc };
			curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeMultiple);
			curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&m);
			
			curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, writeCookies);
			curl_easy_setopt(handle, CURLOPT_HEADERDATA, &cookies);
			
			//curl_easy_setopt(handle, CURLOPT_COOKIE, getCookies().c_str());

			curl_easy_setopt(handle, CURLOPT_URL, url);
			curl_easy_perform(handle);
			cache[string(url)] = newcache;
			//printf("%s\n", newcache.c_str());
			
			curl_easy_cleanup(handle);
		}
		inline bool detectChange(const char* url){
			string urlstr = string(url);
			handle = curl_easy_init();
			curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
			string newcache = "";
			curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlwriteString);
			curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&newcache);

			//curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION,writeCookies);
			//curl_easy_setopt(handle, CURLOPT_HEADERDATA,&cookies);

			curl_easy_setopt(handle, CURLOPT_COOKIE, getCookies().c_str());

			curl_easy_setopt(handle, CURLOPT_URL, url);
			curl_easy_perform(handle);
			curl_easy_cleanup(handle);
			if(cache.count(urlstr)){
				string old=cache[urlstr];
				if(newcache.compare(old)!=0){

					for (int i = 0; i < min(newcache.size(),old.size()); ++i) {
						if(newcache[i]!=old[i]){
							//cout<<newcache[i]<<old[i]<<endl;
						}
					}
					cache[urlstr] = newcache;
					return(true);
				}

			}else{
				cache[urlstr] = newcache;
			}


			return(false);
		}
		inline string getCookies() {
			string out="";
			for (auto c : cookies) {
				out = out+c.first + "=" + c.second + ";";
			}
			return(out);
		}
		inline void download(const char* url,Document* doc) {
			download(url, NULL, doc, false);
		}
		inline void writeString(ofstream* stream,string str) {
			uint32_t l = str.size();
			stream->write((char*)&l,sizeof(l));
			stream->write(str.c_str(), l);
		}
		inline string readString(ifstream* stream) {
			uint32_t l=0;
			stream->read((char*)&l, sizeof(l));
			char* buffer=(char*)malloc(l);
			stream->read(buffer, l);
			return(string(buffer, l));
		}
		inline void saveCache(const char* filename) {
			ofstream out(filename, ios::binary);
			for (auto e : cache) {
				writeString(&out,e.first);
				writeString(&out, e.second);

			}
		}
		inline void loadCache(const char* filename) {
			ifstream ins(filename, ios::binary);
			try{
				while (ins.good()) {
					string url = readString(&ins);
					string data = readString(&ins);
					//printf("%s %s\n", url.c_str(), data.c_str());
					cache[url] = data;
				}
			}catch (const std::exception& e){
				printf("exception %s\n", e.what());
			}
		}
		inline void saveCookies() {

		}
		inline void loadCookies() {

		}

		inline void clearcookies() {
			cookies.clear();
		}
		inline void clearcache() {
			cache.clear();
		}
	};
//#endif // 

}
