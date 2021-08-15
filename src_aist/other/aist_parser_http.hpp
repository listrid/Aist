/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#pragma once

class aist_parser_http_T
{
public:
    aist_parser_http_T()
    {
        m_full_url = NULL;
    }
    ~aist_parser_http_T()
    {
        if(m_full_url)
            free(m_full_url);
    }
    bool IsInit()
    {
        return m_full_url != NULL;
    }
    bool Init(const char* http_url, size_t len = 0)
    {
        if(m_full_url)
        {
            free(m_full_url);
            m_full_url = NULL;
        }
        if(!len)
            len = strlen(http_url);
        if(len < 4)
            return false;
        size_t start_host = 0;
        size_t port = 80;
        if(http_url[0] == 'h')
        {
            if(len > 8 && memcmp(http_url, "http://", 7) == 0)
            {
                start_host = 7;
            }else if(len > 9 && memcmp(http_url, "https://", 8) == 0)
            {
                port = 443;
                start_host = 8;
            }
        }
        size_t stop_host = start_host;
        for(;stop_host < len;stop_host++)
        {
            if(http_url[stop_host] == ':' || http_url[stop_host] == '/')
                break;
        }
        size_t start_url = stop_host;
        if(start_url < len && http_url[start_url] == ':')
        {
            start_url++;
            port = 0;
            while(start_url < len && http_url[start_url] != '/')
            {
                if(http_url[start_url] < '0' || http_url[start_url] > '9')
                    return false;
                port = port*10 + http_url[start_url] -'0';
                start_url ++;
            }
        }
        m_host_len = stop_host - start_host;
        m_url_path_len = len - start_url;

        size_t l_all = len + m_host_len + m_url_path_len + 3;
        if(!m_url_path_len)
            l_all += 1;
        m_full_url = (char*)malloc(l_all);
        m_host = &m_full_url[len+1];
        m_url_path  = &m_host[m_host_len+1];

        m_full_url_len = len;
        memcpy(m_full_url, http_url, len);
        m_full_url[len] = 0;

        memcpy(m_host, &http_url[start_host], m_host_len);
        m_host[m_host_len] = 0;
        if(!m_url_path_len)
        {
            m_url_path_len = 1;
            m_url_path[0] = '/';
            m_url_path[1] = 0;
        }else{
            memcpy(m_url_path, &http_url[start_url], m_url_path_len);
            m_url_path[m_url_path_len] = 0;
        }
        m_port = port;
        return true;
    }
    char*  m_full_url;
    size_t m_full_url_len;

    char*  m_url_path;
    size_t m_url_path_len;

    char*  m_host;
    size_t m_host_len;
    size_t m_port;
};






