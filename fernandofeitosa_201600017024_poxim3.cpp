/*
Author: Luis Fernando Feitosa
FernandoFeitosa_201600017024_poxim3.ccp
17/08/2018 17h
 */

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#define REG 63 // total de registradores

typedef uint32_t uint32;
typedef uint64_t uint64;
using namespace std;
typedef struct
{
    bool valid;
    int idade;
    uint32 id;
    uint32 p[4];
} Tassociacao;

typedef struct grupo_cache
{
    Tassociacao grupo[2];
} Tgrupo;

typedef struct
{
    Tgrupo cache_linha[8];
} Tcache;

void rotuloMin(uint32 Reg, char *operador, int i);

void rotuloMai(uint32 Reg, char *operador, int i);
uint32 op_Ldb(uint32 Ryf, uint32 IM16, vector<u_int32_t> Mem);
uint32 obtemByte(uint32 palavra, uint32 lerPosicao);
uint32 op_Ldw(uint32 Ryf, uint32 IM16, vector<u_int32_t> Mem);
bool readFile(ifstream &ifile, char const *argv);
bool writeFile(ofstream &file, char const *argv);
void inicializarCache(Tcache *cache);
bool mapMemoria(uint32 Posicao);
uint32 consultarCache(uint32 reg, bool tipoCache, Tcache *cache, float *hit, float *miss, vector<u_int32_t> Mem, ofstream &output);
//uint32 consultarCacheI(uint32 reg, bool tipoCache, Tcache *cache, float *hit, float *miss, vector<u_int32_t> Mem, ofstream &output);
uint32 escreverCache(uint32 reg, Tcache *cache, float *hit, float *miss, vector<u_int32_t> Mem, ofstream &output);
void imprimirConsulta(uint32 reg, string modo, string busca, string tipo, string status, Tcache *cache, ofstream &output);
void tabulacao9(ofstream &output);
void imprimirRotulo(uint32 reg, string modo, string busca, string tipo, uint32 linha, ofstream &output);
void verCache(Tcache *cache, ofstream &output);
void imprimirEnd(float D_Hit, float D_Miss, float I_Hit, float I_Miss, ofstream &output);
void imprimirStart(ofstream &output);
void imprimirCache(bool set, string status, uint32 linha, Tcache *cache, ofstream &output);
void incrementarIdadeCache(Tcache *cache);





//ajuste
//funcao principal do simulador POXIM
int main(int argc, char const *argv[])
{
    // Ilustrando uso de argumentos de programa
    cout << "ARGS = " << argc << endl;
    cout << "PROGRAMA = " << argv[0] << endl;
    //argv[1] = "poxim.hex"; argv[2] = "poxim.out";
    //argv[1] = "1_limits.hex";   argv[2] = "1_limits.out";
    // argv[1] = "testePasm.hex"; argv[2] = "testePasm.out";
    // argv[1] = "1_factorial.hex";argv[2] = "1_factorial.out";
    // argv[1] = "1_fibonacci.hex"; argv[2] = "1_fibonacci.out";
    // argv[1] = "1_recursive_factorial.hex"; argv[2] = "1_recursive_factorial.out";
    // argv[1] = "1_recursive_fibonacci.hex"; argv[2] = "1_recursive_fibonacci.out";
    //argv[1] = "2_interruption.hex";    argv[2] = "2_interruption.out";
    //argv[1] = "2_watchdog.hex";    argv[2] = "2_watchdog.out";
    //argv[1] = "2_fpu.hex";    argv[2] = "2_fpu.out";
    //argv[1] = "2_inversao.hex";    argv[2] = "2_inversao.out";
    //argv[1] = "2_hello_world.hex";    argv[2] = "2_hello_world.out";
    //argv[1] = "poxim2.hex";    argv[2] = "poxim2.out";
    //arquivos do poxim3
    argv[1] = "1_factorial.hex";argv[2] = "3_factorial.out";
    //argv[1] = "1_fibonacci.hex"; argv[2] = "3_fibonacci.out";
    //argv[1] = "2_fpu.hex";    argv[2] = "3_fpu2.out";
    //argv[1] = "2_hello_world.hex";    argv[2] = "3_hello_world.out";
    //argv[1] = "2_interruption.hex";    argv[2] = "3_interruption.out";
    //argv[1] = "1_limits.hex";    argv[2] = "3_limits.out";
    // argv[1] = "1_recursive_factorial.hex"; argv[2] = "3_recursive_factorial.out";
    // argv[1] = "1_recursive_fibonacci.hex"; argv[2] = "3_recursive_fibonacci.out";
    //argv[1] = "2_watchdog.hex";    argv[2] = "3_watchdog.out";
    //argv[1] = "3_memory_access.hex";    argv[2] = "3_memory_access.out";
    cout << "ARG1 = " << argv[1] << "\nARG2 = " << argv[2] << endl;
    // Tem que repetir o processo ate o final
    // variaveis auxiliares
    int posicao = 0;
    uint64 temp, termo1, termo2;
    uint32 Rx = 0, Ry = 0, Rz = 0, Rxf = 0, Ryf = 0, IM16 = 0, IM26 = 0, PC = 0;
    uint32 opcode = 0, control_wdog = 0, pc_cacheI = 0; //, pc_cacheD = 0;
    // rotulo para operandos
    char oPz[10] = {}, oPx[10] = {}, oPy[10] = {}, oPrxf[10] = {}, oPryf[10] = {};
    char opz[10] = {}, opx[10] = {}, opy[10] = {}, oprxf[10] = {}, opryf[10] = {};
    char cmndo[40] = {}, res[100] = {}, saida[200] = {}; //, arq[100];
    int sizeCom = (sizeof(cmndo));
    int szRes = (sizeof(res)), szSaida = (sizeof(saida));
    int on_fpu = 0, on_terminal = 0, on_teclado = 0; //on_wdog = 0,
    int blq_wdog = 0, on_wdog = 0;                   // pend_wdog = 1, pend_fpu = 0,
    uint32 control_fpu = 0x0;
    int count_fpu = 0, count_teclado = 0;   // count_cache = 0; //contadores
    int tipo_x = 0, tipo_y = 0, tipo_z = 0; // se 0 entao eh float/IEEE mas se 1 eh inteiro
    uint32 exp_fx = 0, exp_fy = 0, x = 0, y = 0, z = 0, op_fpu = 0;
    // op_fpu -> registra a operacao do fpu
    float *pfx, *pfy, fx, fy, fz; //, aux_f; // manipular operandos do fpu
    //    uint32 *Mem;
    vector<uint32> Mem(100, 0);
    stringstream terminal;
    stringstream teclado;
    string aux = "";
    //    vector<string> terminal(100, "\0");     vector<string> aux(100, "\0");
    // inicializando os registradores com 0.
    uint32 R[REG] = {0};
    int tam = 0, count_terminal = 0; // controlar o tamanho do vector Mem
    int contMem = 0;
    int on = 1;
    //bool teste;

    // olhar o vector em c++
    // Abrindo arquivos
    ifstream input;  // somente para leitura
    ofstream output; // somente para escrita
    //aux = argv[1];
    readFile(input, (argv[1]));
    //aux = argv[2];
    writeFile(output, (argv[2]));
    Tcache cacheI;
    Tcache cacheD;
    inicializarCache(&cacheI);
    inicializarCache(&cacheD);
    //verCache(&cacheI);
    //verCache(&cacheD);

    float count_D_Hit = 0, count_D_Miss = 0, count_I_Hit = 0, count_I_Miss = 0;
    //carregar programa do arquivo.hex

    while (input.good())
    {
        uint32 var = 0x0;
        tam = Mem.size();
        // lê objeto de arquivo
        input >> hex >> var;

        Mem.insert(Mem.begin() + contMem, var);

        // redimensionar o vector Mem
        if (contMem > (tam - 10))
            Mem.resize(contMem + 100);
        // teste.

        contMem++;
    }
    // Depois de carregamento, o vetor contem as instrucoes e dados do hex
    // ajuste para carregar apos a memoria as tres primeiras
    Mem[contMem] = Mem[0];
    contMem++;
    Mem[contMem] = Mem[1];
    contMem++;
    Mem[contMem] = Mem[2];
    //cout << "\tcontMem= " << contMem << "\tMem[]= " << hex << Mem[contMem] << endl;
    //ajuste para depois criar acesso circular
    imprimirStart(output);

    float parar = 0.0; // inicializar variavel de controle de parada
    // funcao principal
    while (on)
    {
        // controle de linhas usado para limitar a execucao e evitar loop infinito
        if (parar == 200000)
            return 0;
        else
        {
            parar++; /*cout << "LINHA = " << parar << "\n";*/
        }
        // cout << count_I_Hit << " / " << count_I_Miss << endl;
        // funcao de watchdog
        if (!blq_wdog && ((R[35] & 0x00000040) == 0x00000040) && (control_wdog == 0x80000000))
        {
            blq_wdog = 1;                 // monitorar a execucao do watchdog
            R[37] = R[32];                // IPC
            R[36] = 0xE1AC04DA;           // CR
            R[32] = 0x00000004;           // PC
            Mem[0x00002020] = 0x00000000; // limpar o watchdog
        }
        if (on_wdog)
        {
            // desabilitar o watchdpg quando contador for zerado
            if (control_wdog > 0x80000000)
            {
                control_wdog--;
            }
            else
            {
                on_wdog = 0;
            }
        }
        // unidade de ponto flutuante (so entra se o watchdog nao estiver rodando)
        if (on_fpu)
        {
            if (!blq_wdog && (count_fpu == 0) && ((R[35] & 0x00000040) == 0x00000040))
            {
                R[37] = R[32];      // IPC
                R[32] = 0x00000008; // PC
                R[36] = 0x01EEE754; // CR
                on_fpu = 0;
                float zInterno;
                // operandos do fpu
                if (tipo_x == 1)
                    fx = *((float *)&Mem[0x2200]);
                else
                    x = Mem[0x2200];
                if (tipo_y == 1)
                    fy = *((float *)&Mem[0x2201]);
                else
                    y = Mem[0x2201];
                if (tipo_z == 1)
                {
                    fz = *((float *)&Mem[0x2202]);
                    zInterno = *((float *)&Mem[0x2202]);
                }
                else
                {
                    z = Mem[0x2202];
                }
                pfx = &fx;
                pfy = &fy;
                control_fpu = Mem[0x00002203];
                switch (op_fpu) // opcoes do fpu
                {
                case 0x0:
                    // cout << "fpu nop" << endl;
                    control_fpu = 0x0;
                    break;
                case 0x01:
                    //        cout << "fpu adicao\t" << endl;
                    tipo_z = 1;
                    fz = fx + fy;
                    z = (*(uint32_t *)(&fz));
                    control_fpu = 0x0;
                    cout << "\tfpu2: fx= " << hex << fx << " , fy= " << hex << fy << " , fz= " << hex << fz << endl;
                    cout << "z= " << z << "x= " << x << "y= " << y << endl;
                    break;
                case 0x02:
                    //        cout << "fpu subtracao" << endl;
                    tipo_z = 1;
                    fz = fx - fy;
                    z = (*(uint32_t *)(&fz));
                    control_fpu = 0x0;
                    break;
                case 0x03:
                    // cout << "fpu multiplicacao" << endl;
                    tipo_z = 1;
                    fz = fx * fy;
                    z = (*(uint32_t *)(&fz)); // ieee
                    control_fpu = 0x0;
                    break;
                case 0x04:
                    if ((*pfy) != 0)
                    {
                        tipo_z = 1;
                        // cout << "\nfpu div normal!\t" << endl;
                        fz = fx / fy;
                        control_fpu = 0x0;
                        z = (*(uint32_t *)(&fz));
                    }
                    else
                    {
                        //    cout << "\nfpu div por zero!\t" << endl;
                        control_fpu = 0x00000020;
                    }
                    break;
                case 0x05:
                    //  cout << "fpu atrib x = z" << endl;
                    x = z;
                    tipo_x = tipo_z;
                    control_fpu = 0x0;
                    break;
                case 0x06:
                    //  cout << "fpu atrib y = z" << endl;
                    y = z;
                    tipo_y = tipo_z;
                    control_fpu = 0x0;
                    break;
                case 0x07:
                    //    cout << "fpu teto[z]" << endl;
                    fz = ceil(zInterno);
                    tipo_z = 0;
                    control_fpu = 0x0;
                    break;
                case 0x08:
                    //    cout << "fpu piso[z]" << endl;
                    fz = floor(zInterno);
                    tipo_z = 0;
                    control_fpu = 0x0;
                    break;
                case 0x09:
                    //    cout << "fpu arred[z]" << *pfz << endl;
                    fz = round(zInterno);
                    tipo_z = 0;
                    control_fpu = 0x0;
                    break;
                default:
                    // OP = Invalido
                    control_fpu = 0x00000020;
                    break;
                }
                if (tipo_x == 1)
                {
                    Mem[0x00002200] = x;
                }
                else
                {
                    Mem[0x00002200] = fx;
                }
                if (tipo_y == 1)
                {
                    Mem[0x00002201] = y;
                }
                else
                {
                    Mem[0x00002201] = fy;
                }
                if (tipo_z == 1)
                {
                    Mem[0x00002202] = z;
                }
                else
                {
                    Mem[0x00002202] = fz;
                }
                Mem[0x00002203] = control_fpu;
            }
            if (count_fpu > 0)
                count_fpu--;
        }
        // tipo de interrupcao
        switch (R[32])
        {
        case 0x04:
            cout << "[HARDWARE INTERRUPTION 1]" << endl;
            output << "[HARDWARE INTERRUPTION 1]" << endl;
            break;
        case 0x08:
            cout << "[HARDWARE INTERRUPTION 2]" << endl;
            output << "[HARDWARE INTERRUPTION 2]" << endl;
            break;
        case 0x0C:
            cout << "[SOFTWARE INTERRUPTION]" << endl;
            output << "[SOFTWARE INTERRUPTION]" << endl;
            break;
        }

        // mantendo o registrado R[0] com valor zero
        saida[0] = 0;
        R[0] = 0x0;
        temp = 0x0;
        // memoria por palavra
        R[33] = (Mem[R[32] >> 2]);
        //R[33] = consultarCache(R[32], 1, &cacheI, &count_I_Hit, &count_I_Miss, Mem, output);
        // Lendo 1 opcode
        opcode = (R[33] & 0xFC000000) >> 26;
        // capturando os operandos nos registradores de uso geral
        Rz = (((R[33] & 0x00020000) >> 12) + ((R[33] & 0x00007C00) >> 10));
        Rx = (((R[33] & 0x00010000) >> 11) + ((R[33] & 0x000003E0) >> 5));
        Ry = (((R[33] & 0x00008000) >> 10) + (R[33] & 0x0000001F));
        Rxf = ((R[33] & 0x000003E0) >> 5);
        Ryf = (R[33] & 0x0000001F);
        IM16 = (R[33] & 0x03FFFC00) >> 10;
        IM26 = (R[33] & 0x03FFFFFF);
        // Decodificando 1 instrucao

        //consultarCacheI(R[32], &cacheI, &count_I_Hit, &count_I_Miss, Mem, output);
        switch (opcode)
        {
            // Operações aritméticas e lógicas
        case 0x00:
            // nop/add = 000000 / U,R[z]=R[x]+R[y] ->OV
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "add %s,%s,%s", opz, opx, opy);
            // processamento
            if (Rz > REG)
                break;
            temp = (uint64)R[Rx] + (uint64)R[Ry];
            if (temp > 0xffffffff) // registrar a ocorrencia de OV
                R[35] = (R[35] | 0x00000010);
            else
                R[35] = (R[35] & 0xffffffef);
            R[Rz] = (uint32)(temp & 0x00000000ffffffff);
            // res
            snprintf(res, szRes, "FR=0x%08X,%s=%s+%s=0x%08X",
                     R[35], oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x10:
            // addi = 010000 /F,R[x]=R[y]+IM16 ->OV
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "addi %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            temp = (uint64)R[Ryf] + (uint64)IM16;
            if (temp > 0xffffffff) // registrar a ocorrencia de OV
                R[35] = (R[35] | 0x00000010);
            else
                R[35] = (R[35] & 0xffffffef);
            R[Rxf] = (uint32)(temp & 0x00000000ffffffff);
            // res
            snprintf(res, szRes, "FR=0x%08X,%s=%s+0x%04X=0x%08X", R[35],
                     oPrxf, oPryf, IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x01:
            // sub = 000001 /U,R[z]=R[x]-R[y] ->OV
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "sub %s,%s,%s", opz, opx, opy);
            // processamento
            termo1 = R[Rx];
            termo2 = R[Ry];
            temp = termo1 - termo2;
            if (temp > 0x0ffffffff)
            {
                R[35] = (R[35] | 0x0000010);
            }
            else
            {
                R[35] = (R[35] & 0xffffffef);
            }
            R[Rz] = (uint32)(temp & 0x00000000ffffffff);
            // res
            snprintf(res, szRes, "FR=0x%08X,%s=%s-%s=0x%08X", R[35],
                     oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x02:
            // mul = 000010 /U,ER|R[z]=R[x]*R[y] ->OV
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "mul %s,%s,%s", opz, opx, opy);
            // processamento
            if (Rz > REG)
                break;
            termo1 = R[Rx];
            termo2 = R[Ry];
            temp = termo1 * termo2;
            if (temp > 0xffffffff) // registrar a ocorrencia de OV
                R[35] = (R[35] | 0x00000010);
            else
                R[35] = (R[35] & 0xffffffef);
            R[34] = (temp >> 32);
            R[Rz] = (uint32)(temp & 0x00000000ffffffff);
            // res
            snprintf(res, szRes, "FR=0x%08X,ER=0x%08X,%s=%s*%s=0x%08X",
                     R[35], R[34], oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x03:
            // div = 000011 /U,ER=R[x]mod R[y],R[z]=R[x]/R[y] ->OV,ZD
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "div %s,%s,%s", opz, opx, opy);
            // processamento
            if (Rz > REG)
                break;
            if (R[Ry] == 0)
            {
                R[35] = (R[35] | 0x00000008);
                R[34] = 0;
                if ((R[35] & 0x40) == 0x40)
                {
                    R[37] = (R[32] + 0x4);
                    R[36] = 0x1;
                    R[32] = 0x0000000C;
                }
                else
                    R[32] += 4;
            }
            else
            {
                termo1 = R[Rx];
                termo2 = R[Ry];
                temp = termo1 / termo2;
                if (temp > 0xffffffff) // registrar a ocorrencia de OV
                    R[35] = (R[35] | 0x00000010);
                else
                    R[35] = (R[35] & 0xffffffef);
                R[35] = R[35] & 0xfffffff7;
                R[34] = (uint32)(termo1 % termo2);
                R[Rz] = (uint32)temp;
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "FR=0x%08X,ER=0x%08X,%s=%s/%s=0x%08X",
                     R[35], R[34], oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x04:
            // cmp = 000100 /U,EQ=R[x]==R[y],LT=R[x]<R[y],GT=R[x]>R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            snprintf(cmndo, sizeCom, "cmp %s,%s", opx, opy);
            // processamento
            R[35] = (R[35] & 0xfffffff8);
            if (R[Rx] == R[Ry])
            {
                R[35] = (R[35] | 0x00000001);
            }
            if (R[Rx] < R[Ry])
            {
                R[35] = (R[35] | 0x00000002);
            }
            if (R[Rx] > R[Ry])
            {
                R[35] = (R[35] | 0x00000004);
            }
            // res
            snprintf(res, szRes, "FR=0x%08X", R[35]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x05:
            // shl = 000101 /U,ER|R[z]=ER|R[x]<<(y+1)=ER|R[x]*2^y+1
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "shl %s,%s,%u", opz, opx, Ry);
            // processamento
            if (Rz > REG)
                break;
            temp = ((((uint64)R[34]) << 32) | ((uint64)R[Rx]));
            temp = (temp << (Ry + 1));
            R[34] = temp >> 32;
            R[Rz] = (uint32)(temp & 0x00000000ffffffff);
            // res
            snprintf(res, szRes, "ER=0x%08X,%s=%s<<%d=0x%08X", R[34], oPz,
                     oPx, Ry + 1, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x06:
            // shr = 000110 /U,ER|R[z]=ER|R[x]>>(y+1)=ER|R[x]/2^y+1
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "shr %s,%s,%u", opz, opx, Ry);
            // processamento
            if (Rz > REG)
                break;
            temp = ((((uint64)R[34]) << 32) | ((uint64)R[Rx]));
            temp = (temp >> (Ry + 1));
            R[34] = temp >> 32;
            R[Rz] = (uint32)(temp & 0x00000000ffffffff);
            // res
            snprintf(res, szRes, "ER=0x%08X,%s=%s>>%d=0x%08X", R[34], oPz,
                     oPx, Ry + 1, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x07:
            // and = 000111 /U,R[z]=R[x]&&R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "and %s,%s,%s", opz, opx, opy);
            // processamento
            if (Rz < REG)
                R[Rz] = (R[Rx] & R[Ry]);
            // res
            snprintf(res, szRes, "%s=%s&%s=0x%08X", oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x08:
            // not = 001000 /U,R[x]=!R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            snprintf(cmndo, sizeCom, "not %s,%s", opx, opy);
            // processamento
            if (Rx < REG)
                R[Rx] = ~R[Ry];
            // res
            snprintf(res, szRes, "%s=~%s=0x%08X", oPx, oPy, R[Rx]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x09:
            // or = 001001 /U,R[z]=R[x] v R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "or %s,%s,%s", opz, opx, opy);
            // processamento
            if (Rz < REG)
                R[Rz] = (R[Rx] | R[Ry]);
            // res
            snprintf(res, szRes, "%s=%s|%s=0x%08X", oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x0A:
            // xor = 001010 /U,R[z]=R[x] ^ R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMin(Rz, opz, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            rotuloMai(Rz, oPz, 4);
            snprintf(cmndo, sizeCom, "xor %s,%s,%s", opz, opx, opy);
            // processamento
            if (Rz < REG)
                R[Rz] = ((~(R[Rx] & R[Ry])) & (R[Rx] | R[Ry]));
            // res
            snprintf(res, szRes, "%s=%s^%s=0x%08X", oPz, oPx, oPy, R[Rz]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;

        case 0x0B:
            // empilhamento.    // push = 001011 /U,
            // Equivalente às instruções stw e subi
            // empilhando o dado R[y] e decrementa o topo da pilha R[x]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            snprintf(cmndo, sizeCom, "push %s,%s", opx, opy);
            // processamento
            if (Ry == 32)
                temp = (R[Ry] >> 2);
            else
                temp = R[Ry];
            Mem[R[Rx]] = temp;
            R[Rx] = R[Rx] - 1;
            // res
            snprintf(res, szRes, "MEM[%s->0x%08X]=%s=0x%08X", oPx,
                     (R[Rx] + 1) << 2, oPy, (uint32)temp);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x0C:
            // desempilhamento: pop = 001100 /U,
            // Equivalente às instruções addi e ldw
            // desempilhando o dado R[x] e incrmentando o topo da pilha R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rx, opx, 4);
            rotuloMin(Ry, opy, 4);
            rotuloMai(Rx, oPx, 4);
            rotuloMai(Ry, oPy, 4);
            snprintf(cmndo, sizeCom, "pop %s,%s", opx, opy);
            // processamento
            R[Ry] += 1;
            R[Rx] = Mem[(R[Ry])];
            // res
            snprintf(res, szRes, "%s=MEM[%s->0x%08X]=0x%08X", oPx, oPy,
                     (R[Ry] << 2), R[Rx]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x11:
            // subi = 010001 /F,R[x]=R[y]-IM16 ->OV
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "subi %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            termo1 = R[Ryf];
            termo2 = IM16;
            //cout << "termo1=" << hex << termo1 << endl;
            //cout << "termo2=" << hex << termo2 << endl;
            temp = termo1 - termo2;
            if (temp > 0x0ffffffff)
            {
                R[35] = (R[35] | 0x00000010);
            }
            else
            {
                R[35] = (R[35] & 0xffffffef);
            }
            R[Rxf] = temp;
            // res
            // teste
            //cout << "\tsubi rxf= " << hex << R[Rxf] << endl;
            snprintf(res, szRes, "FR=0x%08X,%s=%s-0x%04X=0x%08X", R[35],
                     oPrxf, oPryf, IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x12:
            // muli = 010010 /F,ER|R[x]=R[y]*IM16 ->OV
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "muli %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            if (Rxf > 0x0 && Rxf < REG)
            {
                termo1 = R[Ryf];
                termo2 = (uint32)IM16;
                temp = (uint64)termo1 * (uint64)termo2;
                if (temp > 0xffffffff) // registrar a ocorrencia de OV
                    R[35] = (R[35] | 0x0010);
                else
                    R[35] = (R[35] & 0x000f);
                R[34] = (temp >> 32);
                R[Rxf] = (uint32)(temp & 0x00000000ffffffff);
            }
            // res
            snprintf(res, szRes, "FR=0x%08X,ER=0x%08X,%s=%s*0x%04X=0x%08X",
                     R[35], R[34], oPrxf, oPryf, IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x13:
            // divi = 010011 /F,ER=R[y]mod IM16,R[x]=R[y]/IM16 ->OV,ZD
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "divi %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            if (Rxf > REG)
                break;
            if (IM16 == 0)
            {
                R[35] = R[35] | 0x00000008;
                R[34] = 0x0;
                if ((R[35] & 0x40) == 0x40)
                {
                    R[37] = (R[32] + 0x4);
                    R[36] = 0x1;
                    R[32] = 0x0000000C;
                }
                else
                    R[32] += 4;
            }
            else
            {
                termo1 = R[Ryf];
                termo2 = (uint32)IM16;
                temp = (uint64)termo1 / (uint64)termo2; // OV
                // registrar a ocorrencia de OV
                if (temp > 0xffffffff)
                {
                    R[35] = (R[35] | 0x0010);
                }
                else
                {
                    R[35] = (R[35] & 0x006f);
                }
                R[35] = R[35] & 0xfffffff7;
                R[34] = termo1 % termo2;
                //cout << "R[34]= " << hex << R[34] << endl;
                R[Rxf] = (uint32)temp;
                //cout << "R[Rxf]= " << hex << R[Rxf] << endl;
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "FR=0x%08X,ER=0x%08X,%s=%s/0x%04X=0x%08X",
                     R[35], R[34], oPrxf, oPryf, IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;

            break;
        case 0x14:
            // cmpi = 010100 /F,EQ=R[x]==IM16,LT=R[x]<IM16,GT=R[x]>IM16
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            snprintf(cmndo, sizeCom, "cmpi %s,%u", oprxf, IM16);
            // processamento
            R[35] = R[35] & 0xfffffff8;
            if (R[Rxf] < IM16)
                R[35] = (R[35] | 0x00000002);
            else if (R[Rxf] > IM16)
                R[35] = (R[35] | 0x00000004);
            else
                R[35] = (R[35] | 0x00000001);
            // res
            snprintf(res, szRes, "FR=0x%08X", R[35]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x15:
            // andi = 010101 /F,R[x]=R[y]&&IM16
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "andi %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            if (Rxf > 0x0 && Rxf < REG)
                R[Rxf] = (R[Ryf] & IM16);
            // res
            snprintf(res, szRes, "%s=%s&0x%04X=0x%08X", oPrxf, oPryf, IM16,
                     R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x16:
            // noti = 010110 /F,R[x]=!IM16
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "noti %s,%u", oprxf, IM16);
            // processamento
            if (Rxf > 0x0 && Rxf < REG)
                R[Rxf] = ~IM16;
            // res
            snprintf(res, szRes, "%s=~0x%04X=0x%08X", oPrxf, IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x17:
            // ori = 010111 /F,R[x]=R[y] v IM16
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "ori %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            if (Rxf > 0x0 && Rxf < REG)
                R[Rxf] = R[Ryf] | IM16;
            // res
            snprintf(res, szRes, "%s=%s|0x%04X=0x%08X", oPrxf, oPryf, IM16,
                     R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x18:
            // xori = 011000 /F,R[x]=R[y] xor IM16
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "xori %s,%s,%u", oprxf, opryf, IM16);
            // processamento
            if (Rxf < REG)
                R[Rxf] = ((~(R[Ryf] & IM16)) & (R[Ryf] | IM16));
            // res
            snprintf(res, szRes, "%s=%s^0x%04X=0x%08X", oPrxf, oPryf, IM16,
                     R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
            // Operações de acesso a memória
        case 0x19:
            // ldw  = 011001 /F,R[x]=Mem[(R[y]+IM16)] // <<2]
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "ldw %s,%s,0x%04X", oprxf, opryf, IM16);
            // processamento

            switch ((R[Ryf] + IM16))
            {
            case 0x2020:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            case 0x2200:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            case 0x2201:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            case 0x2202:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            case 0x2203:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            case 0x888A:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            case 0x888B:
                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
                break;
            default:
                //cout << "oi R[Rxf]=" << R[Rxf] << endl;
                                R[Rxf] = op_Ldw(R[Ryf], IM16, Mem);
//                R[Rxf] = consultarCache((R[Ryf] + IM16), 0, &cacheD, &count_D_Hit, &count_D_Miss, Mem, output);
                //imprimirConsulta(PC, modo, busca, tipo, set, status, age, cache, output);
                //cout << "oi R[Rxf]=" << R[Rxf] << endl;
                break;
            }

            // res
            //testes
            /*
            output << "LDW." << "\ttemp = " << hex << temp;
            output << "\tMem[(temp)]= " << hex << Mem[(temp)];
            output << "\tposicao= " << hex << posicao;
            output << "\tR[Rxf]=" << hex << R[Rxf] << endl;*/
            snprintf(res, szRes, "%s=MEM[(%s+0x%04X)<<2]=0x%08X",
                     oPrxf, oPryf, IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x1B:
            // ldb = 011011 /F,R[x]=Mem[(R[y]+IM16)]
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "ldb %s,%s,0x%04X", oprxf, opryf, IM16);
            // processamento
            switch ((R[Ryf] + IM16))
            {
            case 0x2020:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            case 0x2200:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            case 0x2201:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            case 0x2202:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            case 0x2203:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            case 0x888A:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            case 0x888B:
                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
                break;
            default:
                //cout << "oi R[Rxf]=" << R[Rxf] << endl;
                                R[Rxf] = op_Ldb(R[Ryf], IM16, Mem);
//                R[Rxf] = obtemByte( consultarCache((R[Ryf] + IM16) >> 2, 0, &cacheD, &count_D_Hit, &count_D_Miss, Mem, output), (R[Ryf] + IM16));
                //imprimirConsulta(PC, modo, busca, tipo, set, status, age, cache, output);
                //cout << "oi R[Rxf]=" << R[Rxf] << endl;
                break;
            }

            // teste
            // output << "LDB." << "\ttemp = " << hex << temp << "\tMem[(temp)]= " << hex << Mem[(temp)];
            // output << "\tposicao= " << hex << posicao << "\tR[Rxf]=" << hex << R[Rxf];
            // output << "\ttermo2=" << hex << termo2 << endl;
            // res
            snprintf(res, szRes, "%s=MEM[%s+0x%04X]=0x%02X", oPrxf, oPryf,
                     IM16, R[Rxf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n", PC, cmndo, res);

            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x1C:
            // stb = 011100 /F,Mem[(R[x]+IM16)]=R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "stb %s,0x%04X,%s",
                     oprxf, IM16, opryf);
            // processamento
            temp = ((R[Rxf] + IM16));
            posicao = temp % 4;
            temp = temp >> 2;
            // redimensionar o vector Mem
            tam = (int)temp;
            if (tam >= (int)Mem.size())
                Mem.resize(tam + 100);
            R[Ryf] = (R[Ryf] & 0x000000ff);
            switch (posicao)
            {
            case 0:
                termo2 = ((Mem[temp] & 0x00ffffff) | (R[Ryf] << 24));
                break;
            case 1:
                termo2 = ((Mem[temp] & 0xff00ffff) | (R[Ryf] << 16));
                break;
            case 2:
                termo2 = ((Mem[temp] & 0xffff00ff) | (R[Ryf] << 8));
                break;
            default:
                termo2 = ((Mem[temp] & 0xffffff00) | R[Ryf]);
                break;
            }

            switch (temp)
            {
            case 0x2020:
                control_wdog = (termo2 & 0xffffffff);
                on_wdog = 1;
                Mem[temp] = termo2;
                break;
            case 0x2200:
                Mem[temp] = termo2; //x
                break;
            case 0x2201:
                Mem[temp] = termo2; //y
                break;
            case 0x2202:
                Mem[temp] = termo2; //z
                break;
            case 0x2203:
                // calculo de expoente da fpu
                Mem[temp] = termo2; // control
                control_fpu = Mem[temp];
                on_fpu = 1; // ativar fpu
                op_fpu = (control_fpu & 0x01f);
                if (op_fpu == 4 && Mem[0x00002201] == 0)
                    count_fpu = 1;
                if (op_fpu > 0 && op_fpu < 5)
                {
                    // calcular expoentes
                    // verificar se eh ieee: fx = *(float*)&x;
                    if (tipo_x == 1)
                    {
                        x = *((float *)&Mem[0x00002200]);
                    }
                    else
                    {
                        x = Mem[0x00002200];
                    }
                    if (tipo_y == 1)
                    {
                        y = *((float *)&Mem[0x00002201]);
                    }
                    else
                    {
                        y = Mem[0x00002201];
                    }
                    fx = x;
                    fy = y;
                    pfx = &fx;
                    pfy = &fy;
                    // extracao dos expoentes para calcular o count_fpu
                    exp_fx = *(int *)(pfx);
                    exp_fx = ((exp_fx)&0x7f800000) >> 23;
                    exp_fx = exp_fx - 0x7f;
                    exp_fy = *(int *)(pfy);
                    exp_fy = ((exp_fy)&0x7f800000) >> 23;
                    exp_fy = exp_fy - 0x7f;
                    count_fpu = (exp_fx - exp_fy) + 1;
                }
                else
                {
                    count_fpu = 1;
                }
                break;
            case 0x2222:
                if (posicao == 2)
                {
                    //      cout << "STB2" << endl;
                    if (isascii(R[Ryf]))
                        teclado << (char)(R[Ryf]);
                    else
                        teclado << (int)(R[Ryf]) << " ";
                    count_teclado++;
                    on_teclado = 1;
                }
                if (posicao == 3)
                {
                    //    cout << "STB3" << endl;
                    if (isascii(R[Ryf]))
                        terminal << (char)(R[Ryf]);
                    else
                        terminal << (int)(R[Ryf]) << " ";
                    count_terminal++;
                    on_terminal = 1;
                }
                Mem[temp] = termo2;
                break;
            default:
                Mem[temp] = termo2;
                break;
            }
            // res
            snprintf(res, szRes, "MEM[%s+0x%04X]=%s=0x%02X",
                     oPrxf, IM16, oPryf, R[Ryf]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            escreverCache((R[Rxf] + IM16), &cacheD, &count_D_Hit, &count_D_Miss, Mem, output);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x1A:
            // stw = 011010 /F,Mem[(R[x]+IM16)<<2]=R[y]
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            snprintf(cmndo, sizeCom, "stw %s,0x%04X,%s", oprxf, IM16, opryf);
            // processamento
            temp = (R[Rxf] + IM16);
            posicao = temp % 4;
            // redimensionar o vector Mem
            tam = (int)temp;
            if (tam >= (int)Mem.size())
                Mem.resize(tam + 100);
            switch (temp)
            {
            case 0x2020:
                Mem[temp] = (R[Ryf] & 0xffffffff);
                control_wdog = Mem[temp];
                on_wdog = 1;
                break;
            case 0x2200:
                Mem[temp] = R[Ryf]; // x
                break;
            case 0x2201:
                Mem[temp] = R[Ryf]; // y
                break;
            case 0x2202:
                Mem[temp] = R[Ryf]; // z
                break;
            case 0x2203:
                // calculo de expoente da fpu
                Mem[temp] = R[Ryf]; // control
                control_fpu = Mem[temp];
                on_fpu = 1; // ativar fpu
                op_fpu = (control_fpu & 0x01f);
                if (op_fpu == 4 && Mem[0x00002201] == 0)
                    count_fpu = 1;
                if (op_fpu > 0 && op_fpu < 5)
                {
                    // calcular expoentes
                    // verificar se eh ieee: fx = *(float*)&x;
                    if (tipo_x == 1)
                        x = *((float *)&Mem[0x00002200]);
                    else
                        x = Mem[0x00002200];
                    if (tipo_y == 1)
                        y = *((float *)&Mem[0x00002201]);
                    else
                        y = Mem[0x00002201];
                    fx = x;
                    fy = y;
                    pfx = &fx;
                    pfy = &fy;
                    // extracao dos expoentes para calcular o count_fpu
                    exp_fx = *(int *)(pfx);
                    exp_fx = ((exp_fx)&0x7f800000) >> 23;
                    exp_fx = exp_fx - 0x7f;
                    exp_fy = *(int *)(pfy);
                    exp_fy = ((exp_fy)&0x7f800000) >> 23;
                    exp_fy = exp_fy - 0x7f;
                    count_fpu = (exp_fx - exp_fy) + 1;
                }
                else
                {
                    count_fpu = 1;
                }
                break;
            case 0x2222:
                if (posicao == 2)
                {
                    //teste.   cout << "STW2" << endl;
                    if (isascii(R[Ryf]))
                        teclado << (char)(R[Ryf]);
                    else
                        teclado << (int)(R[Ryf]) << " ";
                    count_teclado++;
                    on_teclado = 1;
                }
                if (posicao == 3)
                {
                    //teste.cout << "STW3" << endl;
                    if (isascii(R[Ryf]))
                        terminal << (char)(R[Ryf]);
                    else
                        terminal << (int)(R[Ryf]) << " ";
                    count_terminal++;
                    on_terminal = 1;
                }
                Mem[temp] = R[Ryf];
                break;
            default:
                Mem[temp] = R[Ryf];
                break;
            }
            //testes
            /*
            output << "STW." << "\ttemp=" << hex << temp;
            output << "\tMem[(temp)]=" << hex << Mem[(temp)];
            output << "\tposicao=" << hex << posicao;
            output << "\tR[Ryf]=" << hex << R[Ryf] << endl;*/
            // res
            snprintf(res, szRes, "MEM[(%s+0x%04X)<<2]=%s=0x%08X",
                     oPrxf, IM16, oPryf, Mem[temp]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     R[32], cmndo, res);
            escreverCache(temp << 2, &cacheD, &count_D_Hit, &count_D_Miss, Mem, output);
            cout << saida;
            output << saida;
            R[32] += 4;
            break;
        case 0x20: // desvio incondicional
            // bun  = 100000 /S,PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bun 0x%08X", IM26);
            // processamento
            R[32] = IM26 << 2;
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x21: // desvios condicionais
            // beq = 100001 /S,EQ->PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "beq 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000001) == 0x01)
            {
                R[32] = IM26 << 2;
            }
            else
            {
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x22:
            // blt = 100010 /S,LT->PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "blt 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000002) == 0x02)
            {
                R[32] = IM26 << 2;
                // R[33] = Ry;
            }
            else
            {
                //
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x23:
            // bgt = 100011 /S,GT->PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bgt 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000004) == 0x04)
            {
                R[32] = IM26 << 2;
            }
            else
            {
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x24:
            // bne = 100100 /S,!EQ->PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bne 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000001) == 0x0)
            {
                R[32] = IM26 << 2;
                // R[33] = Ry;
            }
            else
            {
                //
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x25:
            // ble = 100101 /S,LT v EQ ->PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "ble 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000002) == 0x02 || (R[35] & 0x00000001) == 0x01)
            {
                R[32] = IM26 << 2;
                // R[33] = Ry;
            }
            else
            {
                //
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x26:
            // bge = 100110 /S,GT v EQ ->PC=IM26<<2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bge 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000004) == 0x04 || (R[35] & 0x00000001) == 0x01)
            {
                R[32] = IM26 << 2;
                // R[33] = Ry;
            }
            else
            {
                //
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x27:
            // bzd = 100111 /S, se ZD −→ PC = IM26 << 2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bzd 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000008) == 0x08)
            {
                R[32] = IM26 << 2;
            }
            else
            {
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x28:
            // bnz = 101000 /S, se ¬ZD −→ PC = IM26 << 2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bnz 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000008) == 0x0)
            {
                R[32] = IM26 << 2;
            }
            else
            {
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x29:
            // biv = 101001 /S, se IV −→ PC = IM26 << 2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "biv 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000020) == 0x00000020)
            {
                R[32] = IM26 << 2;
            }
            else
            {
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x2A:
            // bni = 101010 /S, se ¬IV −→ PC = IM26 << 2
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "bni 0x%08X", IM26);
            // processamento
            if ((R[35] & 0x00000020) == 0x0)
            {
                R[32] = IM26 << 2;
            }
            else
            {
                R[32] += 4;
            }
            // res
            snprintf(res, szRes, "PC=0x%08X", R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x30:
            // chamada de subrotina
            // call = 110000 /F, R[xf] = (PC + 4) >> 2; PC = (R[yf] + IM16) << 2;
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            //temp = IM16-(IM16%4);
            snprintf(cmndo, sizeCom, "call %s,%s,0x%04X", oprxf, opryf, IM16); //(uint32)temp);
            // processamento
            if (Rxf > 0x0 && Rxf < REG)
                R[Rxf] = (R[32] + 4) >> 2;
            R[32] = ((R[Ryf] + IM16) << 2);
            // res
            snprintf(res, szRes,
                     "%s=(PC+4)>>2=0x%08X,PC=(%s+0x%04X)<<2=0x%08X",
                     oPrxf, R[Rxf], oPryf, IM16, R[32]); //(uint32)temp, (uint32)(temp<<2));
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x31:
            // retorno de subrotina
            // ret = 110001 /F, PC = R[x] << 2;
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            snprintf(cmndo, sizeCom, "ret %s", oprxf);
            // processamento
            R[32] = R[Rxf] << 2;
            // res
            snprintf(res, szRes, "PC=%s<<2=0x%08X", oPrxf, R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x32:
            // isr = 110010 /F, R[x] = IPC << 2, R[y] = CR, PC = IM16 << 2;
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMin(Ryf, opryf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            rotuloMai(Ryf, oPryf, 4);
            //snprintf(saida, szSaida, "%s[0x%08X]\t", saida, PC);
            snprintf(cmndo, sizeCom, "isr %s,%s,0x%04X",
                     oprxf, opryf, IM16);
            // processamento
            //cout << "IPC= " << hex << R[37] << "\tCR= " << hex << R[36] << "\tPC= " << hex << IM16 << endl;
            R[Rxf] = R[37] >> 2;
            R[Ryf] = R[36];
            R[32] = IM16 << 2;
            //alteracao para teste
            //R[35] = 0x0;
            // res
            snprintf(res, szRes, "\t%s=IPC>>2=0x%08X,%s=CR=0x%08X,PC=0x%08X",
                     oPrxf, R[Rxf], oPryf, R[Ryf], R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x33:
            // reti = 110011 /F, PC = R[x] << 2;
            // instrucao
            PC = R[32];
            rotuloMin(Rxf, oprxf, 4);
            rotuloMai(Rxf, oPrxf, 4);
            snprintf(cmndo, sizeCom, "reti %s", oprxf);
            // processamento
            R[32] = R[Rxf] << 2;
            if (blq_wdog == 1) //verifica se
            {
                blq_wdog = 0;
            }
            else
            {
                on_fpu = 0;
            }
            // res
            snprintf(res, szRes, "PC=%s<<2=0x%08X", oPrxf, R[32]);
            snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                     PC, cmndo, res);
            cout << saida;
            output << saida;
            break;
        case 0x3F:
            // Operação de interrupção
            // int = 111111 /S
            // Se IM26 == 0, finalizar
            // Se IM26 != 0 −→ CR = IM26, PC = 0x0000000C
            // instrucao
            PC = R[32];
            snprintf(cmndo, sizeCom, "int %u", IM26);
            // processamento
            if (IM26 == 0x0)
            {
                R[32] = 0x0;
                R[36] = IM26;
                on = 0; // off
                // res
                snprintf(res, szRes, "CR=0x%08X,PC=0x%08X",
                         R[36], R[32]);
                snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s",
                         PC, cmndo,
                         res);
                cout << saida << endl;
                output << saida << endl;
                //cout << "teste de terminal" << endl;
                if (on_terminal == 1 || on_teclado == 1)
                {
                    cout << "[TERMINAL]" << endl;
                    output << "[TERMINAL]" << endl;
                }

                if (on_teclado == 1)
                {
                    cout << teclado.str() << endl;
                    output << teclado.str() << endl;
                }

                if (on_terminal == 1)
                {
                    cout << terminal.str() << endl;
                    output << terminal.str() << endl;
                }

                //cout << "visualizar cache de dados" << endl;
                //verCache(&cacheD);
                //cout << "visualizar cache de instrucoes" << endl;
                //verCache(&cacheI);
                imprimirEnd(count_D_Hit, count_D_Miss, count_I_Hit, count_I_Miss, output);
            }
            else
            {
                R[37] = (R[32] + 0x4); // ipc
                R[36] = IM26;          // cr
                R[32] = 0x0000000C;    // pc
                // res
                snprintf(res, szRes, "CR=0x%08X,PC=0x%08X", R[36], R[32]);
                snprintf(saida, szSaida, "[0x%08X]\t%-19s\t%s\n",
                         PC, cmndo, res);
                output << saida;
                cout << saida;
            }
            break;
        default:
            // OP = Invalido
            R[35] = (R[35] | 0x00000020);
            R[37] = (R[32] + 0x4);
            R[36] = (R[32] >> 2);
            snprintf(saida, szSaida, "[INVALID INSTRUCTION @ 0x%08X]", R[32]);
            cout << saida << endl;
            output << saida << endl;
            R[32] = 0x0000000C;
            break;
        }
    }
    //free(Mem);
    return 0;
}

void rotuloMin(uint32 Reg, char *operador, int i)
{
    switch (Reg)
    {
    case 32:
        snprintf(operador, i, "pc");
        break;
    case 33:
        snprintf(operador, i, "ir");
        break;
    case 34:
        snprintf(operador, i, "er");
        break;
    case 35:
        snprintf(operador, i, "fr");
        break;
    case 36:
        snprintf(operador, i, "cr");
        break;
    case 37:
        snprintf(operador, i, "ipc");
        break;
    default:
        snprintf(operador, i, "r%u", Reg);
        break;
    }
}

void rotuloMai(uint32 Reg, char *operador, int i)
{
    switch (Reg)
    {
    case 32:
        snprintf(operador, i, "PC");
        break;
    case 33:
        snprintf(operador, i, "IR");
        break;
    case 34:
        snprintf(operador, i, "ER");
        break;
    case 35:
        snprintf(operador, i, "FR");
        break;
    case 36:
        snprintf(operador, i, "CR");
        break;
    case 37:
        snprintf(operador, i, "IPC");
        break;
    default:
        snprintf(operador, i, "R%u", Reg);
        break;
    }
}

//  Inicializando o/s Registradores
//  0-31  - Registradores de propósito geral. R[0]=0.
//  R[31] - usado como auxiliar nas operacoes aritmeticas
//  32/PC - Contador de programa.
//  33/IR - Registrador de instrução.
//  34/ER - Registrador de extensão.
//  35/FR - Registrador de sinalização.
//  6/IE - 5/IV - 4/OV - 3/ZD - 2/GT - 1/LT - 0/EQ
//  36/CR -
//  37/IPC -
//
// Igualdade: EQ (0 - A 6= B, 1 - A = B)
// Menor que: LT (0 - A ≥ B, 1 - A < B)
// Maior que: GT (0 - A ≤ B, 1 - A > B)
// Divisão por zero: ZD (0 - B 6= 0, 1 - B = 0)
// Overflow: OV (0 - Sem overflow, 1 - Com overflow)
// funcoes arredondamento: ceil floor   round
// float fx, fy, fz;
// uint32_t *pfx = &fx;
// *pfx  guarda end ieee754, fazer mascara para pegar expoente;

void incrementarIdadeCache(Tcache *cache)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        //inicializar a cache de instrucoes
        for (int k = 0; k < 2; k++)
        {
            if ((*cache).cache_linha[i].grupo[k].valid == true)
                (*cache).cache_linha[i].grupo[k].idade = (*cache).cache_linha[i].grupo[k].idade + 1;
        }
    }
}
void imprimirCache(bool set, string status, uint32 linha, Tcache *cache, ofstream &output)
{
    int age = (*cache).cache_linha[linha].grupo[set].idade;
    stringstream saida;
    saida << "\tSET=" << (set == 0 ? "0" : "1") << ":STATUS=" << status << ",AGE=" << age << ",DATA=0x";
    saida.width(8);
    saida.fill('0');
    saida << uppercase << hex << (*cache).cache_linha[linha].grupo[set].p[0];
    saida << "|0x";
    saida.width(8);
    saida.fill('0');
    saida << uppercase << hex << (*cache).cache_linha[linha].grupo[set].p[1];
    saida << "|0x";
    saida.width(8);
    saida.fill('0');
    saida << uppercase << hex << (*cache).cache_linha[linha].grupo[set].p[2];
    saida << "|0x";
    saida.width(8);
    saida.fill('0');
    saida << uppercase << hex << (*cache).cache_linha[linha].grupo[set].p[3];
    cout << saida.str() << endl;
    output << saida.str() << endl;
}
void imprimirStart(ofstream &output)
{
    stringstream saida;
    saida << "[START OF SIMULATION]" << endl;
    cout << saida.str();
    output << saida.str();
}
void imprimirEnd(float D_Hit, float D_Miss, float I_Hit, float I_Miss, ofstream &output)
{

    stringstream saida;
    saida << "[END OF SIMULATION]" << endl;
    saida << "[CACHE]" << endl;
    saida << " D_hit_rate: " << D_Hit << " (" << ((D_Hit * 100) / (float)(D_Hit + D_Miss)) << "%)," << endl;
    saida << " I_hit_rate: " << I_Hit << " (" << ((I_Hit * 100) / (float)(I_Hit + I_Miss)) << "%)," << endl;
    cout << saida.str();
    output << saida.str();
}
void verCache(Tcache *cache, ofstream &output)
{

    stringstream saida;
    int i;
    for (i = 0; i < 8; i++)
    {
        saida << "associacao" << i << endl;
        //inicializar a cache de instrucoes
        for (int k = 0; k < 2; k++)
        {
            saida << "grupo" << k << ":" << endl;
            saida << "\tvalid=" << (*cache).cache_linha[i].grupo[k].valid;
            saida << "\tidade=" << (*cache).cache_linha[i].grupo[k].idade;
            saida << "\tid=" << (*cache).cache_linha[i].grupo[k].id;
            for (int j = 0; j < 4; j++)
            {
                saida << "\tp" << j << "=" << (*cache).cache_linha[i].grupo[k].p[j];
            }
            cout << saida.str() << endl;
            output << saida.str() << endl;
        }
    }
}
void imprimirRotulo(uint32 reg, string modo, string busca, string tipo, uint32 linha, ofstream &output)
{
    int esp1, esp2;
    busca == "hit " ? esp1 = 12 : esp1 = 13;
    busca == "hit " ? esp2 = 20 - esp1 : esp2 = 20 - 13;
    if (modo == "write_")
        esp2--;
    stringstream saida;
    saida << "[0x";
    saida.width(8);
    saida.fill('0');
    saida << uppercase << hex << reg;
    saida << "]\t";
    saida.width(esp1);
    saida.fill(' ');
    saida << left << modo + busca + tipo + "->";
    saida.width(esp2);
    saida.fill(' ');
    saida << left << linha;
    cout << saida.str();
    output << saida.str();
}
void tabulacao9(ofstream &output)
{
    stringstream saida;
    saida << "\t\t\t\t\t\t\t\t\t";
    cout << saida.str();
    output << saida.str();
}
void imprimirConsulta(uint32 reg, string modo, string busca, string tipo, string status, Tcache *cache, ofstream &output)
{
    bool set = 0;
    uint32 linha = (reg & 0x00000070) >> 4;
    imprimirRotulo(reg, modo, busca, tipo, linha, output);
    (*cache).cache_linha[linha].grupo[set].valid ? status = "VALID" : status = "INVALID";
    imprimirCache(set, status, linha, &(*cache), output);
    (*cache).cache_linha[linha].grupo[!set].valid ? status = "VALID" : status = "INVALID";
    tabulacao9(output);
    imprimirCache(!set, status, linha, &(*cache), output);
}
uint32 escreverCache(uint32 reg, Tcache *cache, float *hit, float *miss, vector<u_int32_t> Mem, ofstream &output)
{
    incrementarIdadeCache(&(*cache));
    //cout << "teste cache Instrucao " << endl;
    //reg *= 4;
    uint32 palavra = (reg & 0x0C) >> 2;
    uint32 linha = (reg & 0x70) >> 4;
    uint32 id = (reg & 0xFFFFFF80);
    uint32 posicao = (reg >> 2);
    posicao = posicao - palavra;
    if (palavra != 0) reg = (reg - palavra);
    bool set = 0;
    int c[2];
    c[0] = (*cache).cache_linha[linha].grupo[0].idade;
    c[1] = (*cache).cache_linha[linha].grupo[1].idade;

    bool v01, v02, acertou = 0;
    v01 = (*cache).cache_linha[linha].grupo[set].valid;
    v02 = (*cache).cache_linha[linha].grupo[!set].valid;
    for (int i = 0; i < 2; i++)
    {
        if ((*cache).cache_linha[linha].grupo[i].valid == 1)
        {

            if ((*cache).cache_linha[linha].grupo[i].id == id)
            {
                (*hit) = ((*hit) + 1);
                (*cache).cache_linha[linha].grupo[i].idade = 0;
                imprimirConsulta(reg, "write_", "hit ", "D", "status", &(*cache), output);
                //  cout << (*cache).cache_linha[linha].grupo[i].p[palavra] << endl;
                return (*cache).cache_linha[linha].grupo[i].p[palavra];
            }
            //cout << "linha valida e id igual no grupo0" << endl;
            //  cout << "linha= " << i << endl;
        }
    }
    imprimirConsulta(reg, "write_", "miss ", "D", "status", &(*cache), output);
    (*miss) = ((*miss) + 1);

    if ((*cache).cache_linha[linha].grupo[0].valid == 0)
    {
        for (int i = 0; i < 4; i++)
        {
            (*cache).cache_linha[linha].grupo[0].p[i] = Mem[posicao];
            //    cout << "1cacheD=" << (*cache).cache_linha[linha].grupo[0].p[i] << "Mem[reg>>2]=" << Mem[posicao] << endl;
            posicao = posicao + 1;
        }
        (*cache).cache_linha[linha].grupo[0].valid = true;
        (*cache).cache_linha[linha].grupo[0].idade = 0;
        (*cache).cache_linha[linha].grupo[0].id = id;

        //cout << "linha 3" << endl;
    }
    else if ((*cache).cache_linha[linha].grupo[1].valid == 0)
    {
        for (int i = 0; i < 4; i++)
        {
            (*cache).cache_linha[linha].grupo[1].p[i] = Mem[posicao];
            //    cout << "1cacheD=" << (*cache).cache_linha[linha].grupo[0].p[i] << "Mem[reg>>2]=" << Mem[posicao] << endl;
            posicao = posicao + 1;
        }
        (*cache).cache_linha[linha].grupo[1].valid = true;
        (*cache).cache_linha[linha].grupo[1].idade = 0;
        (*cache).cache_linha[linha].grupo[1].id = id;
    }
    else
    {
        // buscar na memoria e subescrever o mais antigo na cache
        set = (c[0] > c[1] ? 0 : 1);
        for (int i = 0; i < 4; i++)
        {
            (*cache).cache_linha[linha].grupo[set].p[i] = Mem[posicao];
            //      cout << "3cacheD=" << (*cache).cache_linha[linha].grupo[set].p[i] << "Mem[reg>>2]=" << Mem[posicao] << endl;
            posicao = posicao + 1;
        }
        (*cache).cache_linha[linha].grupo[set].valid = true;
        (*cache).cache_linha[linha].grupo[set].idade = 0;
        (*cache).cache_linha[linha].grupo[set].id = id;
    }
    //cout << "Mem[reg>>2]=" << hex << Mem[reg >> 2] << endl;
    return Mem[reg >> 2];
}

uint32 consultarCache(uint32 reg, bool tipoCache, Tcache *cache, float *hit, float *miss, vector<u_int32_t> Mem, ofstream &output)
{
    //cache tipoCache: se 0 de instrucao se 1 de dados
    if (tipoCache == 0) reg *= 4;
    incrementarIdadeCache(&(*cache));
    //cout << "teste cache Instrucao " << endl;
    string tipo;
    (tipoCache == 0)? tipo="D": tipo="I";
    uint32 palavra = (reg & 0x0C) >> 2;
    uint32 linha = (reg & 0x70) >> 4;
    uint32 id = (reg & 0xFFFFFF80);
    uint32 posicao = (reg >> 2);
    posicao = posicao - palavra;
    bool set = 0;
    int c[2];
    c[0] = (*cache).cache_linha[linha].grupo[0].idade;
    c[1] = (*cache).cache_linha[linha].grupo[1].idade;
    //cout << "\tlinha0=" << linha << "\tid=" << id << "\tpalavra=" << palavra << "\treg=" << hex << reg << endl;
    //cout << "\tmem[reg>>2]=" << hex << Mem[posicao] << "\tposicao=" << hex << posicao << endl;
    for (int i = 0; i < 2; i++)
    {
        if ((*cache).cache_linha[linha].grupo[i].valid == 1)
        {
            (*hit) = ((*hit) + 1);
            if ((*cache).cache_linha[linha].grupo[i].id == id)
            {

                (*cache).cache_linha[linha].grupo[i].idade = 0;
                imprimirConsulta(reg, "read_", "hit ", tipo, "status", &(*cache), output);
                //  cout << (*cache).cache_linha[linha].grupo[i].p[palavra] << endl;
                return (*cache).cache_linha[linha].grupo[i].p[palavra]; 
            }
            //cout << "linha valida e id igual no grupo0" << endl;
            //  cout << "linha= " << i << endl;
        }
    }

    imprimirConsulta(reg, "read_", "miss ", tipo, "status", &(*cache), output);

    if ((*cache).cache_linha[linha].grupo[0].valid == 0)
    {
        (*miss) = ((*miss) + 1);

        for (int i = 0; i < 4; i++)
        {
            (*cache).cache_linha[linha].grupo[0].p[i] = Mem[posicao];
            //    cout << "1cacheD=" << (*cache).cache_linha[linha].grupo[0].p[i] << "Mem[reg>>2]=" << Mem[posicao] << endl;
            posicao = posicao + 1;
        }
        (*cache).cache_linha[linha].grupo[0].valid = true;
        (*cache).cache_linha[linha].grupo[0].id = id;
        //(*cache).cache_linha[linha].grupo[0].idade = 0;
        //cout << "linha 3" << endl;
    }
    else
    {
        // buscar na memoria e subescrever o mais antigo na cache
        set = (c[0] > c[1] ? 0 : 1);
        for (int i = 0; i < 4; i++)
        {
            (*cache).cache_linha[linha].grupo[set].p[i] = Mem[posicao];
            //      cout << "3cacheD=" << (*cache).cache_linha[linha].grupo[set].p[i] << "Mem[reg>>2]=" << Mem[posicao] << endl;
            posicao = posicao + 1;
        }
        (*cache).cache_linha[linha].grupo[set].valid = true;
        (*cache).cache_linha[linha].grupo[set].id = id;
        //(*cache).cache_linha[linha].grupo[set].idade = 0;
        //imprimirConsulta(reg, "read_", "miss ", "D", 0, "status", c[set], &(*cache), output);
    }
    //cout << "Mem[reg>>2]=" << hex << Mem[reg >> 2] << endl;
    return Mem[reg >> 2];
}

void inicializarCache(Tcache *cache)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        //inicializar a cache de instrucoes
        for (int k = 0; k < 2; k++)
        {
            (*cache).cache_linha[i].grupo[k].valid = false;
            (*cache).cache_linha[i].grupo[k].idade = 0;
            (*cache).cache_linha[i].grupo[k].id = 0;
            for (int j = 0; j < 4; j++)
            {
                (*cache).cache_linha[i].grupo[k].p[j] = 0;
            }
        }
    }
}

bool writeFile(ofstream &file, char const *argv)
{
    file.open(argv);
    return !(file.fail() || !file.is_open() || !file.good());
}

bool readFile(ifstream &ifile, char const *argv)
{
    ifile.open(argv);
    return !(ifile.fail() || !ifile.is_open() || !ifile.good());
}

uint32 op_Ldw(uint32 Ryf, uint32 IM16, vector<u_int32_t> Mem)
{
    uint64 temp = (Ryf + IM16);
    return (Mem[temp] & 0xffffffff);
}

uint32 obtemByte(uint32 palavra, uint32 lerPosicao)
{
    uint32 posicao = lerPosicao % 4;
    switch (posicao)
    {
    case 0:
        posicao = (palavra & 0xff000000) >> 24;
        //cout << "termo2= " << hex << termo2 << endl;
        break;
    case 1:
        posicao = (palavra & 0x00ff0000) >> 16;
        //cout << "termo2= " << hex << termo2 << endl;
        break;
    case 2:
        posicao = (palavra & 0x0000ff00) >> 8;
        //cout << "termo2= " << hex << termo2 << endl;
        break;
    default:
        posicao = (palavra & 0x000000ff);
        //cout << "termo2= " << hex << termo2 << endl;
        break;
    }
    return (posicao & 0x000000ff);
}

uint32 op_Ldb(uint32 Ryf, uint32 IM16, vector<u_int32_t> Mem)
{

    return obtemByte(Mem[(Ryf + IM16) >> 2], (Ryf + IM16));
}