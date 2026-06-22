#include<atomic>
#include<iostream>
#ifdef __linux__
#include"/media/Estudiante/Pendrive/C++/viggianolib/viggiano"
#else
#include"C:/Users/Mateo Viggiano/Documents/C++/viggianolib/viggiano.hpp"
#endif
#include<memory_resource>
#include<fstream>

#include<thread>
#include<filesystem>

using namespace std;
using namespace mpv;
namespace fs = filesystem;
using fs_char_t=mpv::remove_cv_t<mpv::remove_pointer_t<decltype(mpv::declval<fs::path>().c_str())>>;
struct Simbolo COUNT_IT{
	char simbolo;
	Str<char> binario;
	int apariciones;
	Simbolo(){}
	Simbolo(char simbolo) :simbolo(simbolo), apariciones(1) {}
	bool operator==(const Simbolo& other)const {
		if (this->simbolo == other.simbolo)
			return true;
		else
			return false;
	}
	bool operator!=(const Simbolo& other)const {
		if (this->simbolo != other.simbolo)
			return true;
		else
			return false;
	}
	bool operator>(const Simbolo& other)const {
		if (this->apariciones > other.apariciones)
			return true;
		else
			return false;
	}
	bool operator<(const Simbolo& other)const {
		if (this->apariciones < other.apariciones)
			return true;
		else
			return false;
	}
};

struct Nodo COUNT_IT{
	Vector<char> simbolos;
	int apariciones=0;
	sPtr<Nodo> izquierda;
	sPtr<Nodo> derecha;
	Nodo() :izquierda(nullptr), derecha(nullptr) {}
	Nodo(Simbolo other) :apariciones(other.apariciones), izquierda(nullptr), derecha(nullptr) {
		this->simbolos.push_back(other.simbolo);
	}
};

Vector<Simbolo> contar_simbolos(const char* mensaje, int msg_sz) {
	Vector<Simbolo> lista;
	for (int i = 0; i < msg_sz; i++) {
		Vector<Simbolo>::iterator posicion = lista.find(mensaje[i]);	//busco el indice del simbolo actual en la lista, si no esta devuelve un -1
		if (posicion==lista.end())
			lista.push_back(mensaje[i]);
		else
			posicion->apariciones++;
	}
	return lista;
}
Pair<int, int> menores_posiciones(const Vector<sPtr<Nodo>>& lista) {
	int sz = lista.size(), i;
	Pair<int, int> posiciones(sz - 1, sz - 1);
	for (i = sz - 1; i >= 0; i--) {
		if (lista[i]->apariciones < lista[posiciones.x1]->apariciones)
			posiciones.x1 = i;
	}
	if (posiciones.x1 == sz - 1)
		posiciones.x2 = sz - 2;
	for (i = sz - 1; i >= 0; i--) {
		if (i != posiciones.x1 && lista[i]->apariciones < lista[posiciones.x2]->apariciones)
			posiciones.x2 = i;
	}
	return posiciones;
}
sPtr<Nodo> crear_arbol(Vector<sPtr<Nodo>>& lista) {
	while (lista.size() > 1) {
		Pair<int, int> menores = menores_posiciones(lista);
		sPtr<Nodo> nuevo(make_sPtr<Nodo>());
		nuevo->apariciones = lista[menores.x1]->apariciones + lista[menores.x2]->apariciones;
		nuevo->simbolos = lista[menores.x1]->simbolos + lista[menores.x2]->simbolos;
		nuevo->derecha = lista[menores.x1];
		nuevo->izquierda = lista[menores.x2];
		if (menores.x1 < menores.x2) {
			lista.del_at(menores.x2);
			lista.del_at(menores.x1);
		}
		else {
			lista.del_at(menores.x1);
			lista.del_at(menores.x2);
		}
		lista.insert(lista.begin(), nuevo);
	}
	return lista[0];
}
void asignar_valores_binarios(sPtr<Nodo> raiz, Vector<Simbolo>& caracteres) {
	int sz = caracteres.size();
	for (int i = 0; i < sz; i++) {
		sPtr<Nodo> puntero(raiz);
		while (puntero->simbolos.size() > 1) {
			if (puntero->izquierda->simbolos.contains(caracteres[i].simbolo)) {
				if (puntero->izquierda->apariciones >= puntero->derecha->apariciones)
					caracteres[i].binario += "0";
				else
					caracteres[i].binario += "1";
				puntero = puntero->izquierda;
			}
			else {
				if (puntero->derecha->apariciones > puntero->izquierda->apariciones)
					caracteres[i].binario += "0";
				else
					caracteres[i].binario += "1";
				puntero = puntero->derecha;
			}
		}
	}
}
Str<char> codificar(const char* mensaje, int size, const Vector<Simbolo>& simbolos) {
	Str<char> mensaje_codificado;
	for (int i = 0; i < size; i++) {
		mensaje_codificado += simbolos.find(mensaje[i])->binario;
	}
	return mensaje_codificado;
}
unsigned char pot(unsigned char base, unsigned char exp) {
	unsigned char res = 1;
	for (unsigned char i = 0; i < exp; i++)
		res *= base;
	return res;
}
char toBool(char bit) {
	if (bit == '0')return 0;
	if (bit == '1')return 1;
	else exit(10);
}
char bin_to_dec(const Str<>& bits) {
	char num = 0, exp = 0;
	for (char i = 7; i >= 0; i--, exp++)
		num += toBool(bits[i]) * (unsigned char)pot(2, exp);
	return num;
}
Vector<char> compress(Str<>& binario) {
	while (binario.size() % 8 != 0)
		binario += '0';
	Vector<char> compressed;
	for (size_t i = 0; i < binario.size(); i += 8) {
		compressed.push_back(bin_to_dec(binario.substr(i, i + 8)));
	}
	return compressed;
}
void writeSymbols(const Vector<Simbolo>& simbolos, ofstream& file) {
	for (auto s : simbolos) {
		file.put(s.simbolo);
		file.put((unsigned char)s.binario.size());
		Vector<char> bytes = compress(s.binario);
		for (auto b : bytes)
			file.put(b);
	}
}
void encode(fs::path src, fs::path dest) {
	ifstream inputFile(src, ios::in | ios::binary | ios::ate);
	if (inputFile.fail()) {
		cout << "No se pudo abrir el archivo\n";
		inputFile.close();
		return;
	}
	unsigned int size = inputFile.tellg();
	inputFile.seekg(0);
	char* mensaje = new char[size];
	for (unsigned i = 0; i < size; i++) {
		char c;
		inputFile.get(c);
		mensaje[i] = c;
	}
	inputFile.close();
	Vector<Simbolo> simbolos = contar_simbolos(mensaje, size);
	int cant_simbolos = simbolos.size();
	if (cant_simbolos == 0 || cant_simbolos == 1) {
		cout << "El archivo no se puede comprimir" << endl;
		return;
	}
	simbolos.sort<mpv::greater<Simbolo>>();
	Vector<sPtr<Nodo>> nodos_por_enlazar;
	for (int i = 0; i < cant_simbolos; i++) {
		//nodos_por_enlazar.append(new Nodo(simbolos[i]));
		nodos_por_enlazar.emplace_back(make_sPtr<Nodo>(simbolos[i]));
	}
	sPtr<Nodo> raiz = crear_arbol(nodos_por_enlazar);
	asignar_valores_binarios(raiz, simbolos);
	Str<char> mensaje_codificado = codificar(mensaje, size, simbolos);
	delete[] mensaje;
	fs::path dest_table = dest;
	ofstream file(dest_table.concat(".table"), ios::out | ios::binary);
	if (file.fail()) {
		cout << "error al abrir " << dest_table << "\n";
		file.close();
		return;
	}
	file.write((char*)&size, sizeof(unsigned int));
	writeSymbols(simbolos, file);
	file.close();
	file.open(dest.concat(".compressed"), ios::out | ios::binary);
	if (file.fail()) {
		cout << "error al abrir " << dest << "\n";
		file.close();
		return;
	}
	Vector<char> compressed = compress(mensaje_codificado);
	file.write(compressed.get_array(), compressed.size());
	file.close();

}
void tab(short n) {
	while (n--)
		cout << "\t";
}

List<thread> threads;
fs::path folder, created_folder;
void traverse(fs::path fname, short tabs = 0) {
	Str<fs_char_t> dest_name = fname.c_str();
	dest_name.replace(Str<fs_char_t>(folder.c_str()), Str<fs_char_t>(created_folder.c_str()), 1);
	tab(tabs);
	if (!fs::is_directory(fs::absolute(fname))) {
		cout << fname.filename() << endl;
		fs::path dest_fname = dest_name.c_str();
		threads.push_back(thread(encode, fname, dest_fname));
	}
	else {
		fs::create_directory(dest_name.c_str());
		cout << fname.filename() << ':' << endl;
		for (fs::directory_entry f : fs::directory_iterator(fname)) {
			traverse(f, tabs + 1);
		}
	}
}
fs::path createFolder(fs::path new_folder) {
	new_folder.concat(".enc");
	if (!fs::is_directory(new_folder))
		return new_folder;
	else {
		int i = 1;
		fs::path new_folder_name = new_folder;
		new_folder_name.concat("(1)");
		while (fs::is_directory(new_folder_name)) {
			new_folder_name = new_folder;
			new_folder_name.concat((Str<>('(') + to_str(i++) + ')').c_str());
		}
		return new_folder_name;
	}
}

int main(int argc, char** argv) {
	{
		if (argc == 2) {
			fs::path fName(argv[1]);
			if (fs::exists(fName)) {
				if (fs::is_directory(fName)) {
					folder = fs::absolute(fName);
					created_folder = createFolder(folder);
					traverse(folder);
					for (thread& t : threads)
						t.join();
				}
				else {
					encode(fs::absolute(fName), fs::absolute(fName));
				}
			}
			else cout << "No se encontro el archivo\n";
		}
		else if (argc == 1) {
			fs::path fName;
			do {
				Str<> name;
				cin >> name;
				if (name == ">exit")return 0;
				fName = name.c_str();
			} while (!fs::exists(fName));
			if (fs::is_directory(fName)) {
				folder = fs::absolute(fName);
				created_folder = createFolder(folder);
				traverse(folder);
				for (thread& t : threads)
					t.join();
			}
			else {
				encode(fs::absolute(fName), fs::absolute(fName));
			}
		}
		else {
			cout << "Solo se acepta 1 o 0 argumentos\n";
		}
		threads.clear();
	}cout << endl << "ObjectCounter: " << ObjectCounter::use();/*	Si el contador de objetos esta activo,
																	tiene que devolver 1 por la lista global.
																	Otro valor significa que hay un error.
																	Si el contador de objetos no esta activo devuelve 0.*/
}