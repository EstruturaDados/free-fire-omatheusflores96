/*  Desafio MESTRE

 Grupo: Higor Nunes Botelho Pereira - 202502347031
        Igor Esteferson Vacario - 202503944202
        Klaydiana da Silva Medeiros - 202502264811
        Matheus de Almeida Flores  - 202502541831
  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_COMP 20
#define NOME_LEN 30
#define TIPO_LEN 20

// --- Definição da struct Componente ---
typedef struct {
    char nome[NOME_LEN];    // nome do item
    char tipo[TIPO_LEN];    // categoria/tipo
    int prioridade;         // 1..10 (menor = menos urgente)
} Componente;

// --- Enumeração para identificar critério de ordenação atual ---
typedef enum {
    NAO_ORDENADO = 0,
    ORDENADO_POR_NOME,
    ORDENADO_POR_TIPO,
    ORDENADO_POR_PRIORIDADE
} OrdemAtual;

// --- Protótipos das funções (modularização) ---
// Operações de I/O e utilitárias
void limparBufferLinha(void);                                      // descarta resto da linha stdin
void lerStringFgets(char *dest, int tamanho, const char *prompt);  // lê string com prompt
void mostrarComponentes(Componente arr[], int n);                  // imprime tabela

// Ordenações (cada uma conta comparações via ponteiro long *)
void bubbleSortNome(Componente arr[], int n, long *comparacoes);   // por nome (alfabético)
void insertionSortTipo(Componente arr[], int n, long *comparacoes);  // por tipo (alfabético)
void selectionSortPrioridade(Componente arr[], int n, long *comparacoes);  // por prioridade

// Busca binária por nome (retorna índice ou -1). Também retorna comparações via ponteiro
int buscaBinariaPorNome(Componente arr[], int n, const char chave[], long *comparacoes);

// Função para medir tempo de execução de uma rotina de ordenação 
typedef void (*SortFunc)(Componente[], int, long*);
double medirTempoExecucao(SortFunc f, Componente arr[], int n, long *comparacoes);

// Funções de menu / cadastro
int cadastrarComponentes(Componente arr[], int nAtual);
void clonarVetor(Componente dest[], Componente src[], int n);      // cópia direta (shallow copy)

// --- Implementações --- 

void limparBufferLinha(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {} // descarta até newline
}

// Lê string com fgets e retira '\n' final; usa prompt
void lerStringFgets(char *dest, int tamanho, const char *prompt) {
    printf("%s", prompt);
    if (fgets(dest, tamanho, stdin) != NULL) {
        size_t ln = strlen(dest);
        if (ln > 0 && dest[ln - 1] == '\n') dest[ln - 1] = '\0'; // remove newline
    } else {
        // Caso fgets falhe, garante string vazia
        dest[0] = '\0';
        limparBufferLinha(); // evita lixo no stdin
    }
}

// Exibe a lista de componentes em formato de tabela
void mostrarComponentes(Componente arr[], int n) {
    if (n == 0) {
        printf("Nenhum componente cadastrado.\n");
        return;
    }
    printf("\nÍndice | Nome                          | Tipo               | Prioridade\n");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("%5d | %-28s | %-18s | %9d\n", i, arr[i].nome, arr[i].tipo, arr[i].prioridade);
    }
    printf("\n");
}

// --- Bubble sort por nome (ordem alfabética crescente) --- 
void bubbleSortNome(Componente arr[], int n, long *comparacoes) {
    *comparacoes = 0;
    if (n <= 1) return;
    bool trocou;
    for (int i = 0; i < n - 1; i++) {
        trocou = false;
        for (int j = 0; j < n - 1 - i; j++) {
            (*comparacoes)++;
            if (strcmp(arr[j].nome, arr[j+1].nome) > 0) {
                // troca simples de structs 
                Componente tmp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = tmp;
                trocou = true;
            }
        }
        if (!trocou) break; // otimização: já está ordenado
    }
}

// --- Insertion sort por tipo (ordem alfabética crescente) --- 
void insertionSortTipo(Componente arr[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 1; i < n; i++) {
        Componente chave = arr[i]; // elemento a inserir
        int j = i - 1;
        // compara tipos e desloca para direita quando necessário
        while (j >= 0) {
            (*comparacoes)++;
            if (strcmp(arr[j].tipo, chave.tipo) > 0) {
                arr[j+1] = arr[j]; // desloca
                j--;
            } else {
                break;
            }
        }
        arr[j+1] = chave; // insere na posição correta
    }
}

// --- Selection sort por prioridade (ordem crescente de prioridade: menor -> maior) ---
void selectionSortPrioridade(Componente arr[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 0; i < n - 1; i++) {
        int idxMin = i;
        for (int j = i + 1; j < n; j++) {
            (*comparacoes)++;
            if (arr[j].prioridade < arr[idxMin].prioridade) {
                idxMin = j;
            }
        }
        if (idxMin != i) {
            Componente tmp = arr[i];
            arr[i] = arr[idxMin];
            arr[idxMin] = tmp; // troca quando necessário
        }
    }
}

// --- Busca binária por nome (assume arr[] ordenado por nome ascendente) --- 
int buscaBinariaPorNome(Componente arr[], int n, const char chave[], long *comparacoes) {
    *comparacoes = 0;
    int esq = 0, dir = n - 1;
    while (esq <= dir) {
        int mid = esq + (dir - esq) / 2; // evita overflow (prática segura)
        (*comparacoes)++;
        int cmp = strcmp(arr[mid].nome, chave);
        if (cmp == 0) {
            return mid; // encontrado
        } else if (cmp < 0) {
            esq = mid + 1; // buscar à direita
        } else {
            dir = mid - 1; // buscar à esquerda
        }
    }
    return -1; // não encontrado
}

// Wrapper para medir tempo em segundos; executa a função de ordenação passada
double medirTempoExecucao(SortFunc f, Componente arr[], int n, long *comparacoes) {
    clock_t inicio = clock();
    f(arr, n, comparacoes);
    clock_t fim = clock();
    return (double)(fim - inicio) / (double)CLOCKS_PER_SEC; // tempo em segundos
}

// Permite cadastrar componentes (até MAX_COMP). Retorna novo tamanho do vetor
int cadastrarComponentes(Componente arr[], int nAtual) {
    if (nAtual >= MAX_COMP) {
        printf("Limite de componentes atingido (%d).\n", MAX_COMP);
        return nAtual;
    }
    int restante = MAX_COMP - nAtual;
    int qtd;
    printf("Quantos componentes deseja cadastrar? (max %d): ", restante);
    if (scanf("%d", &qtd) != 1) {
        printf("Entrada inválida.\n");
        limparBufferLinha();
        return nAtual;
    }
    limparBufferLinha(); // limpa '\n' do scanf
    if (qtd <= 0) return nAtual;
    if (qtd > restante) qtd = restante;

    for (int i = 0; i < qtd; i++) {
        printf("\n--- Componente %d de %d ---\n", nAtual + 1, MAX_COMP);
        lerStringFgets(arr[nAtual].nome, NOME_LEN, "Nome (ex: mochila, ak-47, colete/capacete): ");
        lerStringFgets(arr[nAtual].tipo, TIPO_LEN, "Tipo (ex: suporte, armamento, proteção): ");
        int prio = 0;
        do {
            printf("Prioridade (1 a 10): ");
            if (scanf("%d", &prio) != 1) {
                printf("Entrada inválida. Tente novamente.\n");
                limparBufferLinha();
                prio = 0;
            } else {
                limparBufferLinha();
                if (prio < 1 || prio > 10) {
                    printf("Prioridade deve ser entre 1 e 10.\n");
                }
            }
        } while (prio < 1 || prio > 10);
        arr[nAtual].prioridade = prio;
        nAtual++;
    }
    return nAtual;
}

// Copia vetores (para preservar original se necessário)
void clonarVetor(Componente dest[], Componente src[], int n) {
    for (int i = 0; i < n; i++) dest[i] = src[i]; // copia struct-a-struct
}

// --- Função principal: menu interativo ---
int main(void) {
    Componente componentes[MAX_COMP];
    int n = 0; // número atual de componentes
    OrdemAtual estadoOrdem = NAO_ORDENADO;
    Componente trabalhoBackup[MAX_COMP]; // backup do estado antes da ordenação (se precisar)
    int opcao;

    printf("=== Desafio Código da Ilha - Nível Mestre ===\n\n");

    do {
        printf("Menu:\n");
        printf("1. Cadastrar componentes (até %d)\n", MAX_COMP);
        printf("2. Mostrar componentes\n");
        printf("3. Ordenar por NOME (Bubble Sort)\n");
        printf("4. Ordenar por TIPO (Insertion Sort)\n");
        printf("5. Ordenar por PRIORIDADE (Selection Sort)\n");
        printf("6. Buscar componente-chave por NOME (Busca Binária) [requer ordenação por nome]\n");
        printf("7. Limpar todos componentes\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida. Tente novamente.\n");
            limparBufferLinha();
            continue;
        }
        limparBufferLinha(); // remove '\n' restante

        switch (opcao) {
            case 1:
                n = cadastrarComponentes(componentes, n);
                estadoOrdem = NAO_ORDENADO; // novo cadastro invalida ordem
                break;

            case 2:
                mostrarComponentes(componentes, n);
                break;

            case 3: { // Bubble sort por nome
                if (n == 0) {
                    printf("Nenhum componente para ordenar.\n");
                    break;
                }
                clonarVetor(trabalhoBackup, componentes, n); // salva antes
                long comparacoes = 0;
                double tempo = medirTempoExecucao(bubbleSortNome, componentes, n, &comparacoes);
                printf("\nOrdenação por NOME (Bubble Sort) concluída.\n");
                printf("Comparações: %ld\n", comparacoes);
                printf("Tempo de execução: %.6f segundos\n", tempo);
                estadoOrdem = ORDENADO_POR_NOME;
                mostrarComponentes(componentes, n);
                break;
            }

            case 4: { // Insertion sort por tipo
                if (n == 0) {
                    printf("Nenhum componente para ordenar.\n");
                    break;
                }
                clonarVetor(trabalhoBackup, componentes, n);
                long comparacoes = 0;
                double tempo = medirTempoExecucao(insertionSortTipo, componentes, n, &comparacoes);
                printf("\nOrdenação por TIPO (Insertion Sort) concluída.\n");
                printf("Comparações: %ld\n", comparacoes);
                printf("Tempo de execução: %.6f segundos\n", tempo);
                estadoOrdem = ORDENADO_POR_TIPO;
                mostrarComponentes(componentes, n);
                break;
            }

            case 5: { // Selection sort por prioridade
                if (n == 0) {
                    printf("Nenhum componente para ordenar.\n");
                    break;
                }
                clonarVetor(trabalhoBackup, componentes, n);
                long comparacoes = 0;
                double tempo = medirTempoExecucao(selectionSortPrioridade, componentes, n, &comparacoes);
                printf("\nOrdenação por PRIORIDADE (Selection Sort) concluída.\n");
                printf("Comparações: %ld\n", comparacoes);
                printf("Tempo de execução: %.6f segundos\n", tempo);
                estadoOrdem = ORDENADO_POR_PRIORIDADE;
                mostrarComponentes(componentes, n);
                break;
            }

            case 6: { // Busca binária por nome
                if (n == 0) {
                    printf("Nenhum componente cadastrado.\n");
                    break;
                }
                if (estadoOrdem != ORDENADO_POR_NOME) {
                    printf("A busca binária exige que os componentes estejam ordenados por NOME.\n");
                    printf("Ordene por NOME primeiro (opção 3).\n");
                    break;
                }
                char chave[NOME_LEN];
                lerStringFgets(chave, NOME_LEN, "Digite o nome exato do componente-chave: ");
                long comparacoesBusca = 0;
                int pos = buscaBinariaPorNome(componentes, n, chave, &comparacoesBusca);
                if (pos >= 0) {
                    printf("\nComponente encontrado no índice %d:\n", pos);
                    printf("Nome: %s\nTipo: %s\nPrioridade: %d\n", componentes[pos].nome, componentes[pos].tipo, componentes[pos].prioridade);
                } else {
                    printf("\nComponente NÃO encontrado.\n");
                }
                printf("Comparações na busca: %ld\n\n", comparacoesBusca);
                break;
            }

            case 7:
                printf("Confirma limpar todos componentes? (s/n): ");
                {
                    char resp = getchar();
                    limparBufferLinha();
                    if (resp == 's' || resp == 'S') {
                        n = 0;
                        estadoOrdem = NAO_ORDENADO;
                        printf("Componentes apagados.\n");
                    } else {
                        printf("Operação cancelada.\n");
                    }
                }
                break;

            case 0:
                printf("Saindo... boa sorte na ilha!\n");
                break;

            default:
                printf("Opção inválida. Tente novamente.\n");
        }

    } while (opcao != 0);

    return 0;
}
