#include <XHP.hpp>
#include<string>
using namespace XHP;
CurlCache cache=CurlCache();
int resindex=1;
int main(int argc, char **argv) {


	cache.download("https://www.google.com");


	Document doc=Document();

	doc.addCallback("h3.r", [](XHP::Node* n, void* data) {
		auto alist = n->getByTag("a");
		if (alist.size() > 0) {
			string resurl = alist[0]->attributes["href"];
			resurl = resurl.substr(7, resurl.size()-7);
			resurl = resurl.substr(0, resurl.find("&amp"));

			printf("%i. %s\t %s\n",resindex, alist[0]->text().c_str(), resurl.c_str());
			resindex++;


		}
	},NULL);

	std::string searchquery="google";
	if(argc==2){
		searchquery=argv[1];
	}
	searchquery=url_encode(searchquery);
	cache.download(string("https://www.google.com/search?q="+searchquery).c_str(),&doc);

	return(0);
}
