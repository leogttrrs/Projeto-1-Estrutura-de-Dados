#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <sstream>
#include <vector>
#include <queue>


//Essa estrutura de coordenada é usada para calcular a área que o robô deve limpar 
struct Coordenada {
    int x;
    int y;
};

//  Verifica se a coordenada está apta a ser contabilizada
bool coordenadaValida(int x, int y, int altura, int largura, std::vector<std::vector<bool>>& visitados, std::vector<std::string> matriz) {
    return x >= 0 && x < altura && y >= 0 && y < largura && !visitados[x][y] && matriz[x][y] == '1' ;
}

//  Verifica se a tag é de abertura
bool isOpeningTag(const std::string& tag) {
    return tag[1] != '/';
}


//  Pega o nome de uma tag
std::string pegarNomeTag(const std::string& tag) {
    size_t start = tag[1] == '/' ? 2 : 1;
    size_t end = tag.find('>');
    return tag.substr(start, end - start);
}


//  Verifica se o XML está com as tags aninhadas
//  Retorna True se está tudo certo ou False se é identificado um erro
bool verificarXml(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {  //  Verifica se o arquivo foi aberto corretamente
        return false;
    }

    std::stack<std::string> tagStack;  //  Pilha para armazenar as tags
    std::string line;
    while (std::getline(file, line)) {  //  Loop que percorre todas as linhas
        size_t pos = 0;  //  Posição da linha que está sendo lida
        while (pos < line.size()) {  
            size_t start = line.find('<', pos);  //  Encontra o início de uma tag
            if (start == std::string::npos) break;
            size_t end = line.find('>', start);
            if (end == std::string::npos) break;  // Encontra o fim da tag

            std::string tag = line.substr(start, end - start + 1);  // Extrai a tag
            pos = end + 1;

            if (isOpeningTag(tag)) {
                tagStack.push(pegarNomeTag(tag));  // Empilha a tag de abertura
            } else {
                if (tagStack.empty()) {
                    return false;  //  Tag foi fechada sem ser aberta
                }
                std::string topTag = tagStack.top();
                if (topTag != pegarNomeTag(tag)) {
                    return false;  //  Tag fechada não corresponde à ultima aberta
                }
                tagStack.pop();  //  Desempilha a tag correspondente
            }
        }
    }

    if (!tagStack.empty()) {  // Alguma tag no arquivo não foi fechada
        return false;
    }

    return true;
}


//  Função para processar os cenários do arquivo XML
//  e imprimir a área de limpeza de cada um
void processarCenarios(const std::string& filename) {
    std::ifstream file(filename);

    std::string line;
    bool em_cenario = false;  //  Verifica se um cenário está sendo lido
    std::string conteudo_cenario;
    while (std::getline(file, line)) {  //  Lê as linhas do arquivo
        size_t start = line.find('<');
        if (start != std::string::npos) {
            size_t end = line.find('>', start);
            if (end != std::string::npos) {  // Extrai a substring da linha que representa a tag completa, desde '<' até '>'
                std::string tag = line.substr(start, end - start + 1);  //Nome da tag
                std::string tagName = pegarNomeTag(tag);
                if (tagName == "cenario") {  
                    if (isOpeningTag(tag)) {
                        em_cenario = true;
                        conteudo_cenario.clear();  //  Novo cenario será processado
                    } else {
		        //  Declaração de variáveis para processar o cenário
                        em_cenario = false;
                        std::istringstream cenarioStream(conteudo_cenario);
                        std::string linha_cenario;
                        std::string nome_cenario, matrizstr;
                        int xrobo, yrobo, altura, largura;
                        
                        //  Lê as linhas do cenário (mesmo processo de ler o arquivo)
                        //  Loop identifica as subtags de <cenario>
                        while (std::getline(cenarioStream, linha_cenario)) {
                            size_t start = linha_cenario.find('<');
                            if (start != std::string::npos) {
                                size_t end = linha_cenario.find('>', start);
                                if (end != std::string::npos) {
                                    std::string subtag = linha_cenario.substr(start, end - start + 1);
                                    std::string subtagName = pegarNomeTag(subtag);
                                    size_t conteudo_start = end + 1;
                                    size_t conteudo_end = linha_cenario.find('<', conteudo_start);
                                    std::string conteudo = linha_cenario.substr(conteudo_start, conteudo_end - conteudo_start);
                                    if (subtagName == "nome") {
                                        nome_cenario = conteudo;  // Armazena o nome do cenario

                                    } else if (subtagName == "dimensoes") {
		                        //  Achar as subtags de <dimensoes> e guardar as informações
                                        size_t altura_start = linha_cenario.find("<altura>") + 8;
                                        size_t altura_end = linha_cenario.find("</altura>");
                                        altura = stoi(linha_cenario.substr(altura_start, altura_end - altura_start));
                                        size_t largura_start = linha_cenario.find("<largura>") + 9;
                                        size_t largura_end = linha_cenario.find("</largura>");
                                        largura = stoi(linha_cenario.substr(largura_start, largura_end - largura_start));

                                    } else if (subtagName == "robo") {
		                        //  Achar as subtags de <robo> e guardar as informações
                                        size_t xstart = linha_cenario.find("<x>") + 3;
                                        size_t xend = linha_cenario.find("</x>");
                                        xrobo = std::stoi(linha_cenario.substr(xstart, xend - xstart));

                                        size_t ystart = linha_cenario.find("<y>") + 3;
                                        size_t yend = linha_cenario.find("</y>");
                                        yrobo = std::stoi(linha_cenario.substr(ystart, yend - ystart));

                                    }
                                } 
                            } else {
				    //  Armazenar as linhas da matriz em uma unica
				    //  String. as linhas são separads por " ".
                                    matrizstr += linha_cenario;
                                    matrizstr += " ";
                                }
                        }
                        
                        matrizstr.pop_back();  //  Retirar ultimo " " da matriz
                        
                        // Processando as informações retiradas de <cenario>:
                        
                        int quantidade = 0;  //  Quantidade de casas que vão ser limpas

                        std::istringstream stream(matrizstr);
                        std::string linha;
                        std::vector<std::string> matrizcompleta;
                        while (std::getline(stream, linha, ' ')) {  //  Converte a string em uma matriz verdadeira
                            matrizcompleta.push_back(linha);
                        }
                        
                        //  Matriz que armazena coordenadas visitadas. Essa matriz contem as mesmas dimensões de "matrizcompleta", mas armazena booleanos, todos iniciados como "False"
                        std::vector<std::vector<bool>> visitados(altura, std::vector<bool>(largura, false));
                        std::queue<Coordenada> fila;  //  Fila para processar as coordenadas visitadas pelo robô
                        fila.push({xrobo,yrobo});
                        visitados[xrobo][yrobo] = true;  //  Guarda a coordenada inicial como visitada

                        if (matrizcompleta[xrobo][yrobo] != '0'){  //  Verifica se o robô começa em uma casa limpa
                            while (!fila.empty()) {  //  Loop que realiza a limpeza
                                Coordenada atual = fila.front();  // Identifica a coordenada que está sendo processada
                                fila.pop();  //  Remove coordenada da fila
                                quantidade++;  //  Aumenta a quantidade de casas "limpas"

                                // Verifica os vizinhos
                                std::vector<Coordenada> vizinhos = {
                                    {atual.x - 1, atual.y}, // Cima
                                    {atual.x + 1, atual.y}, // Baixo
                                    {atual.x, atual.y - 1}, // Esquerda
                                    {atual.x, atual.y + 1}  // Direita
                                };

                                for (const auto& vizinho : vizinhos) {
                                    if (coordenadaValida(vizinho.x, vizinho.y, altura, largura, visitados, matrizcompleta)) {
                                        fila.push(vizinho);  //  Se o vizinho for válido, entra na fila
                                        visitados[vizinho.x][vizinho.y] = true;  //  Armazena coordenada como visitada
                                    }
                                }   
                            }   
                        }
                        std::cout << nome_cenario << " " << quantidade << std::endl;  // Output com nome do cenário e a quantidade de casas limpas
                    }
                }
            }
        }
        if (em_cenario) {
            conteudo_cenario += line + "\n";  //Armazena o conteúdo do cenário a cada linha. O conteúdo é resetado ao entrar em um novo cenario
        }
    }

    file.close();  //  Fechar o arquivo
}

int main() {
    char xmlfilename[100];
    std::cin >> xmlfilename;  // Entrada do nome do arquivo a ser analisado

    if (!verificarXml(xmlfilename)) {
        std::cout << "erro" << std::endl;  // Imprime "erro" se "verificarXml" retornar "False"
        return 0;  //  Finaliza o programa
    }

    processarCenarios(xmlfilename);  //  Se não houver erro, processa os cenários

    return 0;  //  Finaliza o programa
}
