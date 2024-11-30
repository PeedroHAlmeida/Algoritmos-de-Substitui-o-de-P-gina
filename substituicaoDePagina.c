/////////////////////////////////////////////////////////
//                                                     //
//               Sistemas Operacionais                 //
//           Escalonamento por prioridades             //
//                                                     //
//  Feito por..: Pedro Henrique de Almeida             //
//  Matricula..: 2022.1.08.045                         //
//  Feito por..: Jorran Luka Andrade dos Santos        //
//  Matricula..: 2022.2.08.001                         //
//  Professor..: Fellipe Guilherme Rey de Souza        //
//  Data.......: 30 de novembro de 2024                //
//                                                     //
/////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Estrutura para representar um processo na memória
typedef struct {
    int id;            // ID do processo
    int size;          // Tamanho do processo
    int R;             // Bit R (Referenciado)
    int M;             // Bit M (Modificado)
    int lastUpdate;    // Último tempo em que os bits foram atualizados
} Processo;

// Variáveis globais para a memória
Processo *memoria;  // Ponteiro para o vetor de processos representando a memória
int tamanhoMemoria; // Tamanho total da memória

// Função para inicializar a memória
void inicializarMemoria(int tamanho) {
    tamanhoMemoria = tamanho; // Define o tamanho da memória
    memoria = (Processo *)malloc(tamanhoMemoria * sizeof(Processo)); // Aloca memória para o vetor
    for (int i = 0; i < tamanhoMemoria; i++) {
        // Inicializa cada bloco como livre
        memoria[i].id = -1; 
        memoria[i].size = 0;
        memoria[i].R = 0;
        memoria[i].M = 0;
        memoria[i].lastUpdate = 0;
    }
    printf("Memória inicializada com %d blocos.\n", tamanhoMemoria);
}

// Função para remover um processo baseado na técnica NUR (Not Recently Used)
void removerProcessoPorNUR() {
    int candidato = -1; // Índice do bloco candidato à remoção
    int menorPrioridade = 4; // Prioridade máxima (R=1, M=1)

    // Itera pela memória para encontrar o processo com menor prioridade
    for (int i = 0; i < tamanhoMemoria; i++) {
        if (memoria[i].id != -1) { // Bloco ocupado
            int prioridadeAtual = memoria[i].R * 2 + memoria[i].M; // Calcula a prioridade (R e M)

            // Verifica se é o de menor prioridade
            if (prioridadeAtual < menorPrioridade) {
                menorPrioridade = prioridadeAtual;
                candidato = i;
            }

            // Se encontrar prioridade mínima (0), para a busca
            if (menorPrioridade == 0) {
                break;
            }
        }
    }

    if (candidato != -1) { // Se encontrou um candidato
        int idRemover = memoria[candidato].id;

        // Remove todos os blocos ocupados pelo processo
        for (int i = 0; i < tamanhoMemoria; i++) {
            if (memoria[i].id == idRemover) {
                memoria[i].id = -1; // Marca como livre
                memoria[i].size = 0;
                memoria[i].R = 0;
                memoria[i].M = 0;
                memoria[i].lastUpdate = 0;
            }
        }

        printf("Processo %d removido da memória para abrir espaço.\n", idRemover);
    } else {
        printf("Erro: Nenhum processo foi encontrado para remoção.\n");
    }
}

// Função para inserir um processo usando a técnica First-Fit
int inserirProcesso(int id, int tamProcesso, int tempoAtual) {
    while (1) {
        // Busca o primeiro espaço contíguo que comporte o processo
        for (int i = 0; i < tamanhoMemoria; i++) {
            int espacoDisponivel = 1;
            for (int j = 0; j < tamProcesso; j++) {
                // Verifica se os blocos consecutivos estão livres
                if (i + j >= tamanhoMemoria || memoria[i + j].id != -1) {
                    espacoDisponivel = 0;
                    break;
                }
            }
            if (espacoDisponivel) { // Se encontrou espaço
                // Preenche os blocos com o processo
                for (int j = 0; j < tamProcesso; j++) {
                    memoria[i + j].id = id;
                    memoria[i + j].size = tamProcesso;
                    memoria[i + j].R = 1; // Inicialmente referenciado
                    memoria[i + j].M = 1; // Inicialmente modificado
                    memoria[i + j].lastUpdate = tempoAtual;
                }
                printf("Processo %d de tamanho %d inserido na memória.\n", id, tamProcesso);
                return 1; // Inserção bem-sucedida
            }
        }

        // Caso não haja espaço, tenta liberar removendo um processo
        printf("Memória cheia. Tentando remover um processo...\n");
        removerProcessoPorNUR();
    }

    printf("Erro: Não foi possível inserir o processo %d.\n", id);
    return 0; // Inserção falhou
}

// Função para atualizar os bits R periodicamente
void atualizarBitsR(int tempoAtual) {
    for (int i = 0; i < tamanhoMemoria; i++) {
        if (memoria[i].id != -1) { // Apenas para blocos ocupados
            if (memoria[i].R == 1) {
                memoria[i].R = 0; // Reseta o bit R
            }
        }
    }
}

// Função para atualizar os bits M periodicamente
void atualizarBitsM(int tempoAtual) {
    for (int i = 0; i < tamanhoMemoria; i++) {
        if (memoria[i].id != -1) { // Apenas para blocos ocupados
            // Reseta o bit M se o processo não foi atualizado há 10 ciclos
            if (memoria[i].M == 1 && memoria[i].lastUpdate + 10 == tempoAtual) {
                memoria[i].M = 0;
            }
        }
    }
}

// Função para exibir o estado atual da memória
void exibirMemoria() {
    printf("\nEstado da memória:\n");
    for (int i = 0; i < tamanhoMemoria; i++) {
        if (memoria[i].id != -1) { // Bloco ocupado
            printf("Bloco %d: ID = %d, R%dM%d, Ultima Atualizacao = %d\n",
                   i, memoria[i].id, memoria[i].R, memoria[i].M, memoria[i].lastUpdate);
        } else {
            printf("Bloco %d: Livre\n", i); // Bloco livre
        }
    }
}

// Função principal
int main() {
    srand(time(NULL)); // Inicializa a semente do gerador de números aleatórios

    int tamanho; 
    int tempoAtual = 0; 
    int id = 0;
    int tamMaxProcesso = 0;
    char c;

    // Loop para garantir entrada válida
    while (1) { 
            printf("Digite o tamanho da memória: ");
            
            if (scanf("%d", &tamanho) != 1) { // Verifica se a entrada é um número
                printf("Entrada inválida! Por favor, insira apenas números.\n");
                while ((c = getchar()) != '\n' && c != EOF);
            } else if (tamanho < 5) { // Verifica se o número é menor que 5
                printf("O tamanho da memória inválido. Digite um número maior ou igual a 5.\n");
            }else{
                break; // Saída do loop se a entrada for válida
            }
        }

    // Tamanho máximo de processo
    tamMaxProcesso = 5;


    inicializarMemoria(tamanho);

    // Loop principal da simulação
    while (1) {
        printf("\nTempo Atual: %d\n", tempoAtual);
        
        // Atualiza bits R a cada 10 ciclos
        if (tempoAtual % 10 == 0) {
            atualizarBitsR(tempoAtual);
        }
        // Atualiza bits M periodicamente
        atualizarBitsM(tempoAtual);

        // 40% de chance de inserir um novo processo
        if ((rand() % 100) < 40) {
            id++;
            inserirProcesso(id, (rand() % tamMaxProcesso) + 1, tempoAtual);
        }

        // Exibe o estado atual da memória
        exibirMemoria();

        // Pausa de 1 segundo para melhor visualização
        sleep(1);

        // Incrementa o tempo atual e evita loops infinitos
        tempoAtual++;
    }

    return 0;
}
