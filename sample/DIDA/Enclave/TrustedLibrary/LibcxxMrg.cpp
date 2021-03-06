#include <libcxx/sstream>
#include <libcxx/string>
#include <libcxx/vector>

#include "../Enclave.h"
#include "Enclave_t.h"

std::vector<char *> samFiles;

unsigned int cntgCount = 0;
unsigned int readCount = 0;
int pCount = 0;
std::string *alnName;

char newline_delim[] = "\n";

void ecall_init_merge(unsigned int maxCt, unsigned int maxRd,
                      unsigned int ps, unsigned char *alName, long len)
{
    cntgCount = maxCt;
    readCount = maxRd;
    pCount = ps;
    alnName = new std::string(reinterpret_cast<char *>(alName));
    printf("Initialized mrg. p : %d, alnName : %s, maxCt: %d:%d, maxRd: %d:%d\n", ps, alName, maxCt,
           cntgCount, maxRd, readCount);
}

char *strcpy(char *destination, const char *source)
{
    // return if no memory is allocated to the destination
    if (destination == NULL)
        return NULL;

    // take a pointer pointing to the beginning of destination string
    char *ptr = destination;

    // copy the C-string pointed by source into the array
    // pointed by destination
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }

    // include the terminating null character
    *destination = '\0';

    // destination is returned by standard strcpy()
    return ptr;
}

void fordMer(const int pNum, const std::string &alignerName)
{
    std::string mapStr(alignerName);

    printf("DStarting merging mrg. p : %d, alnName : %s, maxCt: %d, maxRd: %d\n", pNum, mapStr, cntgCount, readCount);
    printf("Maximum target ID=%d\nTotal number of queries=%d\n", cntgCount, readCount);
    printf("No of sam files : %d", samFiles.size());

    printf("Defining variables...\n");
    std::vector<std::vector<int>> ordList(readCount);

    printf("Done defining vector...\n");

    printf("Defining ints...\n");
    unsigned int readId, bitFg;
    printf("Done defining ints...\n");
    std::string readHead = "";
    std::string headSQ = "";
    printf("First pass starting...");

    char *line = nullptr;
    // First pass, Reading
    int headEnd[pNum];
    for (int i = 0; i < pNum; ++i)
    {
        headEnd[i] = 0;
        char *sam_cpy = (char *)malloc(strlen(samFiles[i]) + 1);
        strcpy(sam_cpy, samFiles[i]);
        line = strtok(sam_cpy, newline_delim);
        while (line != nullptr)
        {
            if (line[0] != '@')
                break;
            ++headEnd[i];
            line = strtok(NULL, newline_delim);
        }
        // inserting SAM info into ordList

        while (line != nullptr)
        {
            char **endptr;
            int read = 0;

            readId = std::strtol(line, endptr, 10);

            ordList[readId].push_back(i + 1);
            line = strtok(NULL, newline_delim);
        }
        delete sam_cpy;
    }

    printf("First pass done\n");

    //Second pass, Writing
    std::string comFile = "";
    std::vector<char *> saved_ptrs(pNum);
    for (int i = 0; i < pNum; i++)
    {
        saved_ptrs[i] = samFiles[i];
    }

    for (int i = 0; i < pNum; ++i)
    {
        //Discarding @
        printf("Pnum %d head ends at %d\n", i, headEnd[i]);
        for (int j = 0; j < headEnd[i]; ++j)
        {
            if (j == 0)
            {
                line = strtok_r(samFiles[i], newline_delim, &saved_ptrs[i]);
            }
            else
            {
                line = strtok_r(NULL, newline_delim, &saved_ptrs[i]);
            }
        }
    }

    printf("Second pass done\n");

    char colChar;
    for (unsigned i = 0; i < readCount; ++i)
    {
        bool samVal = false;
        for (unsigned j = 0; j < ordList[i].size(); ++j)
        {
            line = strtok_r(NULL, newline_delim, &saved_ptrs[ordList[i][j] - 1]);
            char *line_cpy = (char *)malloc(strlen(line) + 1);
            strcpy(line_cpy, line);
            char *endptr;
            readId = std::strtol(line, &endptr, 10);
            colChar = endptr[0];
            char *head = strtok_r(endptr + 1, "\t", &endptr);
            bitFg = std::strtol(endptr, &endptr, 10);

            //printf("ReadId : %d, colChar: %c, head: %s, bitFg:%d\n", readId, colChar, head, bitFg);
            // std::istringstream iss(line);
            // iss>>readId>>colChar>>readHead>>bitFg;

            std::string line_str(line_cpy);
            if (bitFg != 4)
            {
                samVal = true;
                size_t pos = line_str.find_first_of(":");
                comFile.append(line_str.substr(pos + 1, std::string::npos));
                comFile.append("\n");
            }
            delete line_cpy;
        }
        if (!samVal)
        {
            comFile.append(readHead).append("\t4\t*\t0\t0\t*\t*\t0\t0\t*\t*\n");
        }
    }

    //sealing
    size_t plaintext_len = comFile.size() + 1;
    uint8_t *plaintext = (uint8_t *)comFile.c_str();
    size_t sealed_size = sizeof(sgx_sealed_data_t) + plaintext_len;
    uint8_t *sealed_data = (uint8_t *)malloc(sealed_size);
    sgx_status_t status = seal(plaintext, plaintext_len, (sgx_sealed_data_t *)sealed_data, sealed_size);

    //printf("Final SAM : \n\n%s", comFile.c_str());
    std::string aln_sam = "aln.sam";
    
    ocall_print_file((const char *)sealed_data, aln_sam.c_str(), 0);
    //ocall_print_file(comFile.c_str(), aln_sam.c_str(), 1);
}

void ecall_load_sealed_sam(sgx_sealed_data_t *sealed_data, size_t sealed_size, long char_len, int pid)
{
    // do decryption
    char *data = (char *)malloc(char_len + 1);
    uint8_t *plaintext = (uint8_t *)&data;
    uint32_t plaintext_len = char_len + 1;
    unseal(sealed_data, sealed_size, plaintext, plaintext_len);

    if (pid != samFiles.size())
    {
        printf("Something wrong. Expected the sam file from process %d", pid);
    }
    // todo inefficient copy
    char *cpy = new char[char_len];
    for (long i = 0; i < char_len; i++)
    {
        cpy[i] = data[i];
    }
    samFiles.push_back(cpy);
}

void ecall_load_sam(char *data, long char_len, int pid)
{
    if (pid != samFiles.size())
    {
        printf("Something wrong. Expected the sam file from process %d", pid);
    }
    // todo inefficient copy
    char *cpy = new char[char_len];
    for (long i = 0; i < char_len; i++)
    {
        cpy[i] = data[i];
    }
    samFiles.push_back(cpy);
}

void ecall_load_sam_lreads(char *data, long char_len)
{
    samFiles.push_back(data);
}

void ecall_finalize_merge()
{
    fordMer(pCount, *alnName);

    cntgCount = 0;
    readCount = 0;
    samFiles.clear();
}