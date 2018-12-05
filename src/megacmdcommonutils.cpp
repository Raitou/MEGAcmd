/**
 * @file src/megacmdcommonutils.cpp
 * @brief MEGAcmd: Auxiliary methods
 *
 * (c) 2013-2016 by Mega Limited, Auckland, New Zealand
 *
 * This file is part of the MEGAcmd.
 *
 * MEGAcmd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "megacmdcommonutils.h"

#ifdef _WIN32
#else
#include <sys/ioctl.h> // console size
#include <unistd.h>
#endif

#include <iomanip>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <limits.h>
#include <iterator>

using namespace std;

#ifdef _WIN32

//override << operators for wostream for string and const char *

std::wostream & operator<< ( std::wostream & ostr, std::string const & str )
{
    std::wstring toout;
    stringtolocalw(str.c_str(),&toout);
    ostr << toout;

return ( ostr );
}

std::wostream & operator<< ( std::wostream & ostr, const char * str )
{
    std::wstring toout;
    stringtolocalw(str,&toout);
    ostr << toout;
    return ( ostr );
}

//override for the log. This is required for compiling, otherwise SimpleLog won't compile.
std::ostringstream & operator<< ( std::ostringstream & ostr, std::wstring const &str)
{
    std::string s;
    localwtostring(&str,&s);
    ostr << s;
    return ( ostr );
}


#endif


bool canWrite(string path)
{
#ifdef _WIN32
    // TODO: Check permissions
    return true;
#else
    if (access(path.c_str(), W_OK) == 0)
    {
        return true;
    }
    return false;
#endif
}

bool isPublicLink(string link)
{
    if (( link.find("http") == 0 ) && ( link.find("#") != string::npos ))
    {
        return true;
    }
    return false;
}

bool isEncryptedLink(string link)
{
    if (( link.find("http") == 0 ) && ( link.find("#") != string::npos ) && (link.substr(link.find("#"),3) == "#P!") )
    {
        return true;
    }
    return false;
}

bool hasWildCards(string &what)
{
    return what.find('*') != string::npos || what.find('?') != string::npos;
}

std::string &ltrim(std::string &s, const char &c)
{
    size_t pos = s.find_first_not_of(c);
    s = s.substr(pos == string::npos ? s.length() : pos, s.length());
    return s;
}

std::string &rtrim(std::string &s, const char &c)
{
    size_t pos = s.find_last_of(c);
    size_t last = pos == string::npos ? s.length() : pos;
    if (last + 1 < s.length())
    {
        if (s.at(last + 1) != c)
        {
            last = s.length();
        }
    }

    s = s.substr(0, last);
    return s;
}

vector<string> getlistOfWords(char *ptr, bool escapeBackSlashInCompletion, bool ignoreTrailingSpaces)
{
    vector<string> words;

    char* wptr;

    // split line into words with quoting and escaping
    for (;; )
    {
        // skip leading blank space
        while (*(const signed char*)ptr > 0 && *ptr <= ' ' && (ignoreTrailingSpaces || *(ptr+1)))
        {
            ptr++;
        }

        if (!*ptr)
        {
            break;
        }

        // quoted arg / regular arg
        if (*ptr == '"')
        {
            ptr++;
            wptr = ptr;
            words.push_back(string());

            for (;; )
            {
                if (( *ptr == '"' ) || ( *ptr == '\\' ) || !*ptr)
                {
                    words[words.size() - 1].append(wptr, ptr - wptr);

                    if (!*ptr || ( *ptr++ == '"' ))
                    {
                        break;
                    }

                    wptr = ptr - 1;
                }
                else
                {
                    ptr++;
                }
            }
        }
        else if (*ptr == '\'') // quoted arg / regular arg
        {
            ptr++;
            wptr = ptr;
            words.push_back(string());

            for (;; )
            {
                if (( *ptr == '\'' ) || ( *ptr == '\\' ) || !*ptr)
                {
                    words[words.size() - 1].append(wptr, ptr - wptr);

                    if (!*ptr || ( *ptr++ == '\'' ))
                    {
                        break;
                    }

                    wptr = ptr - 1;
                }
                else
                {
                    ptr++;
                }
            }
        }
        else
        {
            while (*ptr == ' ') ptr++;// only possible if ptr+1 is the end

            wptr = ptr;

            char *prev = ptr;
            //while ((unsigned char)*ptr > ' ')
            while ((*ptr != '\0') && !(*ptr ==' ' && *prev !='\\'))
            {
                if (*ptr == '"')
                {
                    while (*++ptr != '"' && *ptr != '\0')
                    { }
                }
                prev=ptr;
                ptr++;
            }
                string newword(wptr, ptr - wptr);
                words.push_back(newword);
        }
    }

    if (escapeBackSlashInCompletion && words.size()> 1 && words[0] == "completion")
    {
        for (int i = 1; i < (int)words.size(); i++)
        {
            replaceAll(words[i],"\\","\\\\");
        }
    }

    return words;
}

bool stringcontained(const char * s, vector<string> list)
{
    for (int i = 0; i < (int)list.size(); i++)
    {
        if (list[i] == s)
        {
            return true;
        }
    }

    return false;
}

char * dupstr(char* s)
{
    char *r;

    r = (char*)malloc(sizeof( char ) * ( strlen(s) + 1 ));
    strcpy(r, s);
    return( r );
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
    {
        return false;
    }
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return;
    }
    size_t start_pos = 0;
    while (( start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

int toInteger(string what, int failValue)
{
    if (what.empty())
    {
        return failValue;
    }
    if (!isdigit(what[0]) && !( what[0] != '-' ) && ( what[0] != '+' ))
    {
        return failValue;
    }

    char * p;
    long l = strtol(what.c_str(), &p, 10);

    if (*p != 0)
    {
        return failValue;
    }

    if (( l < INT_MIN ) || ( l > INT_MAX ))
    {
        return failValue;
    }
    return (int)l;
}

string joinStrings(const vector<string>& vec, const char* delim, bool quoted)
{
    stringstream res;
    if (!quoted)
    {
        std:copy(vec.begin(), vec.end(), ostream_iterator<string>(res, delim));
    }
    else
    {
        for(vector<string>::const_iterator i = vec.begin(); i != vec.end(); ++i)
        {
            res << "\"" << *i << "\"" << delim;
        }
    }
    if (vec.size()>1)
    {
        string toret = res.str();
        return toret.substr(0,toret.size()-strlen(delim));
    }
    return res.str();
}

unsigned int getstringutf8size(const string &str) {
    int c,i,ix,q;
    for (q=0, i=0, ix=int(str.length()); i < ix; i++, q++)
    {
        c = (unsigned char) str[i];

        if (c>=0 && c<=127) i+=0;
        else if ((c & 0xE0) == 0xC0) i+=1;
#ifdef _WIN32
        else if ((c & 0xF0) == 0xE0) i+=2;
#else
        else if ((c & 0xF0) == 0xE0) {i+=2;q++;} //these gliphs may occupy 2 characters! Problem: not always. Let's assume the worse
#endif
        else if ((c & 0xF8) == 0xF0) i+=3;
        else return 0;//invalid utf8
    }
    return q;
}

string getFixLengthString(const string origin, unsigned int size, const char delim, bool alignedright)
{
    string toret;
    size_t printableSize = getstringutf8size(origin);
    size_t bytesSize = origin.size();
    if (printableSize <= size){
        if (alignedright)
        {
            toret.insert(0,size-printableSize,delim);
            toret.insert(size-bytesSize,origin,0,bytesSize);

        }
        else
        {
            toret.insert(0,origin,0,bytesSize);
            toret.insert(bytesSize,size-printableSize,delim);
        }
    }
    else
    {
        toret.insert(0,origin,0,(size+1)/2-2);
        if (size > 3) toret.insert((size+1)/2-2,3,'.');
        if (size > 1) toret.insert((size+1)/2+1,origin,bytesSize-(size)/2+1,(size)/2-1); //TODO: This could break characters if multibyte!  //alternative: separate in multibyte strings and print one by one?
    }

    return toret;
}

string getRightAlignedString(const string origin, unsigned int minsize)
{
    ostringstream os;
    os << std::setw(minsize) << origin;
    return os.str();
}

int getFlag(map<string, int> *flags, const char * optname)
{
    return flags->count(optname) ? ( *flags )[optname] : 0;
}

string getOption(map<string, string> *cloptions, const char * optname, string defaultValue)
{
    return cloptions->count(optname) ? ( *cloptions )[optname] : defaultValue;
}

int getintOption(map<string, string> *cloptions, const char * optname, int defaultValue)
{
    if (cloptions->count(optname))
    {
        int i = defaultValue;
        istringstream is(( *cloptions )[optname]);
        is >> i;
        return i;
    }
    else
    {
        return defaultValue;
    }
}



string sizeProgressToText(long long partialSize, long long totalSize, bool equalizeUnitsLength, bool humanreadable)
{
    ostringstream os;
    os.precision(2);
    if (humanreadable)
    {
        string unit;
        unit = ( equalizeUnitsLength ? " B" : "B" );
        double reducedPartSize = (double)totalSize;
        double reducedSize = (double)totalSize;

        if ( totalSize > 1099511627776LL *2 )
        {
            reducedPartSize = totalSize / (double) 1099511627776ull;
            reducedSize = totalSize / (double) 1099511627776ull;
            unit = "TB";
        }
        else if ( totalSize > 1073741824LL *2 )
        {
            reducedPartSize = totalSize / (double) 1073741824L;
            reducedSize = totalSize / (double) 1073741824L;
            unit = "GB";
        }
        else if (totalSize > 1048576 * 2)
        {
            reducedPartSize = totalSize / (double) 1048576;
            reducedSize = totalSize / (double) 1048576;
            unit = "MB";
        }
        else if (totalSize > 1024 * 2)
        {
            reducedPartSize = totalSize / (double) 1024;
            reducedSize = totalSize / (double) 1024;
            unit = "KB";
        }
        os << fixed << reducedPartSize << "/" << reducedSize;
        os << " " << unit;
    }
    else
    {
        os << partialSize << "/" << totalSize;
    }

    return os.str();
}

string sizeToText(long long totalSize, bool equalizeUnitsLength, bool humanreadable)
{
    ostringstream os;
    os.precision(2);
    if (humanreadable)
    {
        string unit;
        unit = ( equalizeUnitsLength ? " B" : "B" );
        double reducedSize = (double)totalSize;

        if ( totalSize > 1099511627776LL *2 )
        {
            reducedSize = totalSize / (double) 1099511627776ull;
            unit = "TB";
        }
        else if ( totalSize > 1073741824LL *2 )
        {
            reducedSize = totalSize / (double) 1073741824L;
            unit = "GB";
        }
        else if (totalSize > 1048576 * 2)
        {
            reducedSize = totalSize / (double) 1048576;
            unit = "MB";
        }
        else if (totalSize > 1024 * 2)
        {
            reducedSize = totalSize / (double) 1024;
            unit = "KB";
        }
        os << fixed << reducedSize;
        os << " " << unit;
    }
    else
    {
        os << totalSize;
    }

    return os.str();
}

int64_t textToSize(const char *text)
{
    int64_t sizeinbytes = 0;

    char * ptr = (char *)text;
    char * last = (char *)text;
    while (*ptr != '\0')
    {
        if (( *ptr < '0' ) || ( *ptr > '9' ) || ( *ptr == '.' ) )
        {
            switch (*ptr)
            {
                case 'b': //Bytes
                case 'B':
                    *ptr = '\0';
                    sizeinbytes += int64_t(atof(last));
                    break;

                case 'k': //KiloBytes
                case 'K':
                    *ptr = '\0';
                    sizeinbytes += int64_t(1024.0 * atof(last));
                    break;

                case 'm': //MegaBytes
                case 'M':
                    *ptr = '\0';
                    sizeinbytes += int64_t(1048576.0 * atof(last));
                    break;

                case 'g': //GigaBytes
                case 'G':
                    *ptr = '\0';
                    sizeinbytes += int64_t(1073741824.0 * atof(last));
                    break;

                case 't': //TeraBytes
                case 'T':
                    *ptr = '\0';
                    sizeinbytes += int64_t(1125899906842624.0 * atof(last));
                    break;

                default:
                {
                    return -1;
                }
            }
            last = ptr + 1;
        }
        char *prev = ptr;
        ptr++;
        if (*ptr == '\0' && ( ( *prev == '.' ) || ( ( *prev >= '0' ) && ( *prev <= '9' ) ) ) ) //reach the end with a number or dot
        {
            return -1;
        }
    }
    return sizeinbytes;

}

string percentageToText(float percentage)
{
    ostringstream os;
    os.precision(2);
    if (percentage != percentage) //NaN
    {
        os << "----%";
    }
    else
    {
        os << fixed << percentage*100.0 << "%";
    }

    return os.str();
}

unsigned int getNumberOfCols(unsigned int defaultwidth)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns = defaultwidth;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        columns = csbi.srWindow.Right - csbi.srWindow.Left - 1;
    }

    return columns;
#else
    struct winsize size;
    if ( ioctl(STDOUT_FILENO,TIOCGWINSZ,&size) != -1
         || (ioctl(STDIN_FILENO,TIOCGWINSZ,&size) != -1))
    {
        if (size.ws_col > 2)
        {
            return size.ws_col - 2;
        }
    }
#endif
    return defaultwidth;
}

void sleepSeconds(int seconds)
{
#ifdef _WIN32
    Sleep(1000*seconds);
#else
    sleep(seconds);
#endif
}

void sleepMilliSeconds(long milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds *1000);
#endif
}

bool isValidEmail(string email)
{
    return !( (email.find("@") == string::npos)
                    || (email.find_last_of(".") == string::npos)
                    || (email.find("@") > email.find_last_of(".")));
}

string &ltrimProperty(string &s, const char &c)
{
    size_t pos = s.find_first_not_of(c);
    s = s.substr(pos == string::npos ? s.length() : pos, s.length());
    return s;
}

string &rtrimProperty(string &s, const char &c)
{
    size_t pos = s.find_last_not_of(c);
    if (pos != string::npos)
    {
        pos++;
    }
    s = s.substr(0, pos);
    return s;
}

string &trimProperty(string &what)
{
    rtrimProperty(what,' ');
    ltrimProperty(what,' ');
    if (what.size() > 1)
    {
        if (what[0] == '\'' || what[0] == '"')
        {
            rtrimProperty(what, what[0]);
            ltrimProperty(what, what[0]);
        }
    }
    return what;
}

string getPropertyFromFile(const char *configFile, const char *propertyName)
{
    ifstream infile(configFile);
    string line;

    while (getline(infile, line))
    {
        if (line.length() > 0 && line[0] != '#')
        {
            if (!strlen(propertyName)) //if empty return first line
            {
                return trimProperty(line);
            }
            string key, value;
            size_t pos = line.find("=");
            if (pos != string::npos && ((pos + 1) < line.size()))
            {
                key = line.substr(0, pos);
                rtrimProperty(key, ' ');

                if (!strcmp(key.c_str(), propertyName))
                {
                    value = line.substr(pos + 1);
                    return trimProperty(value);
                }
            }
        }
    }

    return string();
}