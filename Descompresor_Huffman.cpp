#include<atomic>
#include<iostream>
#include"F:/C++/viggianolib/viggiano"
#include<windows.h>
#include<fstream>
#include<thread>
#include<filesystem>
#include<memory_resource>
using namespace std;
using namespace mpv;
namespace fs = filesystem;

pmr::synchronized_pool_resource rsrc;
struct Symbol COUNT_IT{
	char symbol;
	Str<> bin;
	Symbol() {}
	Symbol(char s) :symbol(s) {}
};
unsigned char toChar(bool n) {
	if (n == 1) return '1';
	else return '0';
}
Str<> dec_to_bin(unsigned char num) {
	Str<> binary = "";
	while (num != 0) {
		binary.insert(0, toChar(num % 2));
		num /= 2;
	}
	while (binary.len() < 8)
		binary.insert(0, '0');
	return binary;
}
bool is_int(float n) {
	n = n - (int)n;
	if (n == 0) return true;
	else return false;
}
void readSymbols(Vector<Symbol,pmr::polymorphic_allocator<Symbol>>& simbolos, fstream& file, int fsize) {
	for (int i = 4; i < fsize;) {
		char b;
		file.get(b); i++;
		simbolos.append(b);
		file.get(b); i++;
		unsigned char bin_sz = b;
		float n = (float)bin_sz / 8.f;
		if (not is_int(n))
			n = int(n + 1);
		Vector<char,pmr::polymorphic_allocator<char>> bin(&rsrc);
		for (int j = 0; j < n; j++) {
			file.get(b); i++;
			bin.append(b);
		}
		Str<> bits = "";
		for (auto bit : bin)
			bits += dec_to_bin(bit);
		while ((unsigned char)bits.len() > bin_sz)
			bits.del_back();
		simbolos.back().bin = bits;
	}
}
Str<> decompress(const char* compressed, int size) {
	Str<> binary;
	for (int i = 0; i < size; i++)
		binary += dec_to_bin(compressed[i]);
	return binary;
}
void decode(fs::path fpath) {
	Str<> bin;
	fstream file(fpath, ios::in | ios::binary | ios::ate);
	if (file.fail()) {
		cout << "No se pudo abrir " << fpath << endl;
		return;
	}
	int fsize = file.tellg();
	file.seekg(0);
	char* compressed_bin = new char[fsize];
	for (int i = 0; i < fsize; i++)
		file.get(compressed_bin[i]);
	file.close();
	bin = decompress(compressed_bin, fsize);
	file.open(fpath.replace_extension(".table"), ios::in | ios::binary | ios::ate);
	if (file.fail()) {
		cout << "No se pudo abrir " << fpath << endl;
		return;
	}
	fsize = file.tellg();
	file.seekg(0);
	unsigned int msj_len;
	file.read((char*)&msj_len, sizeof(unsigned int));
	Vector<Symbol,pmr::polymorphic_allocator<Symbol>> list(&rsrc);
	readSymbols(list, file, fsize);
	file.close();
	file.open(fpath.replace_extension(""), ios::out | ios::binary);
	if (file.fail()) {
		cout << "No se pudo abrir " << fpath << endl;
		return;
	}
	unsigned int cont = 0;
	for (unsigned long long j = 0; true;) {
		bool error = true;
		for (size_t i = 0; i < list.len(); i++) {
			if (bin.continueswith(list[i].bin, j)) {
				file.put(list[i].symbol);
				j += list[i].bin.len();
				error = false;
				cont++;
				break;
			}
		}
		// for(auto i:list){
		// 	if(bin.continueswith(i.bin,j)){
		// 		file.put(i.symbol);
		// 		j+=i.bin.len();
		// 		error=false;
		// 		cont++;
		// 		break;
		// 	}
		// }
		if (cont == msj_len) {
			//cout<<"\r"<<fpath.filename()<<'<'<<cont<<'/'<<msj_len<<">:SUCCESS\n";
			break;
		}
		if (error) {
			cout << "\nERROR\n";
			return;
		}
	}
	file.close();
	fs::remove(fpath.concat(".compressed"));
	fs::remove(fpath.replace_extension(".table"));
	return;
}
List<thread> threads;
void traverse(fs::path fname, short tabs = 0) {
	Str<> tabstr = Str<>("\t") * tabs;
	if (not fs::is_directory(fname)) {
		if (fname.extension() == ".compressed") {
			cout << tabstr << fname.filename() << endl;
			threads.append(thread(decode, fname));
		}
	}
	else {
		cout << tabstr << fname.filename() << ":\n";
		for (fs::directory_entry f : fs::directory_iterator(fname))
			traverse(f, tabs + 1);
	}
}
int main(int argc, char** argv) {
	{
		if (argc == 1) {
			Str<> filename;
			cin >> filename;
			if (fs::exists(filename.c_str()))
				traverse(fs::absolute(filename.c_str()));
			else
				cout << "El archivo no existe\n";
		}
		else if (argc == 2) {
			if (fs::exists(argv[1]))
				traverse(fs::absolute(argv[1]));
			else
				cout << "El archivo no existe\n";
		}
		else {
			cout << "\nSe debe ingresar maximo 1 argumento\n";
			return 1;
		}
		for (thread& t : threads)
			t.join();
		threads.clear();
	}cout << endl << "ObjectCounter: " << ObjectCounter::use();
}
