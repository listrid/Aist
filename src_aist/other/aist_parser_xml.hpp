/**
* @author:  Egorov Sergey <listrid@yandex.ru>
**/
#pragma once

#include "../aist_mem.h"
#include <string.h>
#include <stdio.h>

class aist_parser_xml_T
{
public:
//     struct xml_attributes
//     {
//         const char* m_name;
//         size_t      m_name_len;
//         const char* m_value;
//         size_t      m_value_len;
//         char        m_separator;
//     };
    struct xml_node
    {
        const char* m_name;
        size_t      m_name_len;
        //xml_attributes* m_attributes;
        //size_t      m_attributes_count;
        const char* m_attributes;
        size_t      m_attributes_len;
        enum
        {
            NODE_EMPTY,    // пустой узел (возможно данные в атрибутах)
            NODE_CHILDS,   // нода содержит узлы
            NODE_DATA,     // нода содержит данные
        }m_type;
        union
        {
            const char* m_data;
            xml_node*   m_childs;
        };
        union
        {
            size_t m_childs_count;
            size_t m_data_size;
        };
    };
private:
    struct xml_parser
    {
        aist_mem_T m_mem;

        char   m_cur_char;
        size_t m_cur_index;
        const char* m_contents;
        size_t      m_length;
        void next()
        {
            if(m_cur_char != '\0' && m_cur_index < m_length)
            {
                m_cur_index ++;
                m_cur_char = m_contents[m_cur_index];
            }else{
                m_cur_char = '\0';
            }
        }
        void skip_space()
        {
            while(m_cur_char == ' ' || m_cur_char == '\r' || m_cur_char == '\n' || m_cur_char == '\t')
                next();
        }
        void skip_name()
        {
            while(m_cur_char != '\0' && m_cur_char != ' ' && m_cur_char != '/' && m_cur_char != '>')
                next();
        }
        void find_end_block()
        {
            while(m_cur_char != '\0' && m_cur_char != '>' && !(m_cur_char == '/' && m_contents[m_cur_index+1] == '>'))
                next();
        }
        xml_parser(const char* xml, size_t xmlLen)
        {
            m_mem.Init(0x1000);
            m_contents = xml;
            m_length   = xmlLen;
            m_cur_index = 0;
            m_cur_char = m_contents[0];
        }
    };

    struct xml_level
    {
        size_t m_nodes_count;
        size_t m_nodes_max;
        xml_node* m_nodes;
    };
    static const int XML_MAX_LEVEL = 128;
    bool ParseAttribute(const char* text, size_t textLen, xml_node* node)
    {//!! пока не потребовалось
        return true;
    }
    bool ParseData(const char* xml, size_t xmlLen)
    {
        xml_parser parser(xml, xmlLen);
        m_xml_header_len = 0;
        m_root_count = 0;

        parser.skip_space();
        if(parser.m_cur_char != '<')
            return false;
        xml_level lvl_noda[XML_MAX_LEVEL];
        aist_os_memset(lvl_noda, 0, sizeof(lvl_noda));
        int level = 0;

        size_t header_pos = parser.m_cur_index;
        if(parser.m_contents[parser.m_cur_index+1] == '?') //skip <? .... ?>
        {
            parser.next();
            parser.next();
            while(parser.m_cur_char != '\0' && parser.m_cur_char != '?')
                parser.next();
            while(parser.m_cur_char != '\0' && parser.m_cur_char != '>')
                parser.next();
            if(parser.m_cur_char != '>')
                return false;
            parser.next();
            m_xml_header = &parser.m_contents[header_pos];
            m_xml_header_len = parser.m_cur_index - header_pos;
        }
        while(parser.m_cur_index < parser.m_length && level >= 0)
        {
            parser.skip_space();
            if(parser.m_cur_char == '\0')
                break;
            if(parser.m_cur_char != '<')
                return false;
            parser.next();
            if(parser.m_cur_char == '!')//comment '<!'
            {
                parser.next();
                while(parser.m_cur_char != '\0' && parser.m_cur_char != '>')
                {
                    while(parser.m_cur_char != '\0' && parser.m_cur_char != '!')
                        parser.next();
                    parser.next();
                }
                if(parser.m_cur_char == '\0')
                    return false;
                parser.next();
                continue;
            }
            if(parser.m_cur_char == '/')//close
            {
                parser.next();
                parser.skip_space();
                size_t start_name = parser.m_cur_index;
                parser.skip_name();
                const char* name = &parser.m_contents[start_name];
                size_t name_len  = parser.m_cur_index - start_name;
                parser.skip_space();
                if(parser.m_cur_char != '>')
                    return false;
                parser.next();
                xml_node* childs_nodes = lvl_noda[level].m_nodes;
                size_t    childs_count = lvl_noda[level].m_nodes_count;
                aist_os_memset(&lvl_noda[level], 0, sizeof(xml_level));
                level--;
                if(level < 0 || lvl_noda[level].m_nodes_count == 0)
                    break;
                xml_node* tmp = &lvl_noda[level].m_nodes[lvl_noda[level].m_nodes_count-1];
                tmp->m_childs = childs_nodes;
                tmp->m_childs_count = childs_count;
                if(name_len != tmp->m_name_len || memcmp(name, tmp->m_name, name_len))
                    return false;
                continue;
            }
            if(lvl_noda[level].m_nodes_count == lvl_noda[level].m_nodes_max)
            {//alloc level node
                xml_node* tmp = lvl_noda[level].m_nodes;
                lvl_noda[level].m_nodes_max += lvl_noda[level].m_nodes_max/2+1;
                lvl_noda[level].m_nodes = (xml_node*)m_mem.Alloc(sizeof(xml_node)*lvl_noda[level].m_nodes_max);
                if(tmp)
                {
                    aist_os_memcpy(lvl_noda[level].m_nodes, tmp, sizeof(xml_node)*lvl_noda[level].m_nodes_count);
                    m_mem.Free(tmp, sizeof(xml_node)*lvl_noda[level].m_nodes_count);
                }
            }
            xml_node* node = &lvl_noda[level].m_nodes[lvl_noda[level].m_nodes_count];
            lvl_noda[level].m_nodes_count++;
            aist_os_memset(node, 0, sizeof(xml_node));
            parser.skip_space();
            size_t start_name = parser.m_cur_index;
            parser.skip_name();
            node->m_name     = &parser.m_contents[start_name];
            node->m_name_len = parser.m_cur_index - start_name;
            parser.skip_space();
            if(parser.m_cur_char == '/' && parser.m_contents[parser.m_cur_index+1] == '>')//пустой
            {
                parser.next();
                parser.next();
                node->m_type = xml_node::NODE_EMPTY;
                continue;
            }
            if(parser.m_cur_char == '\0')
                return false;
            if(parser.m_cur_char != '>')
            {
                size_t start_meta = parser.m_cur_index;
                parser.find_end_block();
                //if(!ParseAtribut(&parser.m_contents[start_meta], parser.m_cur_index - start_meta, node))
                //    return false;
                node->m_attributes =  &parser.m_contents[start_meta];
                node->m_attributes_len = parser.m_cur_index - start_meta;
                if(node->m_attributes_len && node->m_attributes[node->m_attributes_len-1] == ' ')
                    node->m_attributes_len --;
            }
            if(parser.m_cur_char == '/' && parser.m_contents[parser.m_cur_index+1] == '>')//пустой с мета
            {
                parser.next();
                parser.next();
                node->m_type = xml_node::NODE_EMPTY;
                continue;
            }
            parser.next(); // чето есть
            size_t start_data = parser.m_cur_index;
            parser.skip_space();
            if(parser.m_cur_char != '<')
            {//данные
                while(parser.m_cur_char != '\0'  && parser.m_cur_char != '<')
                    parser.next();
                if(parser.m_cur_char == '\0')
                    return false;
                size_t stop_data  = parser.m_cur_index;
                parser.next();
                if(parser.m_cur_char != '/')
                    return false;
                parser.next();
                parser.skip_space();
                size_t start_name = parser.m_cur_index;
                parser.skip_name();
                const char* name = &parser.m_contents[start_name];
                size_t name_len = parser.m_cur_index - start_name;
                if(name_len != node->m_name_len || memcmp(name, node->m_name, name_len))
                    return false;
                parser.skip_space();
                if(parser.m_cur_char != '>')
                    return false;
                parser.next();
                node->m_data = &parser.m_contents[start_data];
                node->m_data_size = stop_data - start_data;
                node->m_type = xml_node::NODE_DATA;
                continue;
            }
            node->m_type = xml_node::NODE_CHILDS;
            if(parser.m_contents[parser.m_cur_index+1] != '/')
            {
                level++;
                if(level >= XML_MAX_LEVEL)
                    return false;
            }
            continue;
        }
        if(level < 0)
            return false;
        m_root       = lvl_noda[0].m_nodes;
        m_root_count = lvl_noda[0].m_nodes_count;
        return true;
    }
    void WriteStop(FILE* f, xml_node* node, bool format)
    {
        fwrite("</", 1, 2, f);
        fwrite(node->m_name, 1, node->m_name_len, f);
        fwrite(">\r\n", 1, format?3:1, f);
    }
    void WriteStart(FILE* f, xml_node* node, bool format)
    {
        fwrite("<", 1, 1, f);
        fwrite(node->m_name, 1, node->m_name_len, f);
        if(node->m_attributes_len)
        {
            fwrite(" ", 1, 1, f);
            fwrite(node->m_attributes, 1, node->m_attributes_len, f);
        }
        if(node->m_type == xml_node::NODE_EMPTY)
        {
            fwrite(" />\r\n", 1, format?5:3, f);
        }else if(node->m_type == xml_node::NODE_DATA && node->m_data_size)
        {
            fwrite(">", 1, 1, f);
            fwrite(node->m_data, 1, node->m_data_size, f);
        }else{
            fwrite(">\r\n", 1, format?3:1, f);
        }
    }
public:
    aist_parser_xml_T()
    {
        m_mem.Init(0x1000);
    }
    ~aist_parser_xml_T()
    {
        m_mem.FreeStorage();
    }
    bool Init(const char* xml, size_t xmlLen = 0)
    {
        if(!xmlLen)
            xmlLen = strlen(xml);
        m_mem.Clear();
        return ParseData(xml, xmlLen);
    }
    const xml_node* find(const xml_node* node, const char* name, size_t nameLen = 0, size_t index = 0)
    {
        if(!node)
            return NULL;
        if(node->m_type != xml_node::NODE_CHILDS)
            return NULL;
        if(!nameLen)
            nameLen = strlen(name);
        for(size_t i = 0; i < node->m_childs_count; i++)
        {
            if(nameLen == node->m_childs[i].m_name_len && memcmp(name, node->m_childs[i].m_name, nameLen) == 0)
            {
                if(!index)
                    return &node->m_childs[i];
                index--;
            }
        }
        return NULL;
    }
    const xml_node* find_root(const char* name, size_t nameLen = 0, size_t index = 0)
    {
        if(!nameLen)
            nameLen = strlen(name);
        for(size_t i = 0; i < m_root_count; i++)
        {
            if(nameLen == m_root[i].m_name_len && memcmp(name, m_root[i].m_name, nameLen) == 0)
            {
                if(!index)
                    return &m_root[i];
                index--;
            }
        }
        return NULL;
    }
    const xml_node* find(const char* name)
    {
        const xml_node* n = NULL;
        size_t start = 0, i = 0;
        while(name[i])
        {
            i++;
            if(name[i] == '/' || name[i] == '\\' || name[i] == '\0')
            {
                if(!n)
                {
                    n = find_root(&name[start], i-start);
                }else{
                    n = find(n, &name[start], i-start);
                }
                if(!n)
                    break;
                start = i+1;
            }
        }
        return n;
    }


    bool Sаve(const char* fileName, bool format)
    {
        FILE* f = fopen(fileName, "wb+");
        if(!f)
            return false;
        if(m_xml_header_len)
            fwrite(m_xml_header, 1, m_xml_header_len, f);
        if(format)
            fwrite("\r\n", 1, 2, f);
        int lvl_index[XML_MAX_LEVEL];
        xml_node* lvl_node[XML_MAX_LEVEL];
        char spase[XML_MAX_LEVEL*3];
        aist_os_memset(spase, ' ', sizeof(spase));
        for(size_t i = 0; i < m_root_count; i++)
        {
            lvl_node[0]  = &m_root[i];
            lvl_index[0] = 0;
            int level = 0;
            while(level >= 0)
            {

                xml_node* node = lvl_node[level];//перейдем на уровень ниже
                if(lvl_index[level] == 0)
                {
                    if(format && level)
                        fwrite(spase, 1, level*3, f);
                    WriteStart(f, node, format);
                }
                if(node->m_type != xml_node::NODE_CHILDS)
                {
                    if(node->m_type != xml_node::NODE_EMPTY)
                        WriteStop(f, node, format);
                    level--;
                    continue;
                }
                if(lvl_index[level] == node->m_childs_count)
                {
                    if(format && level)
                        fwrite(spase, 1, level*3, f);
                    WriteStop(f, node, format);
                    level--;
                    continue;
                }
                lvl_index[level + 1] = 0;
                lvl_node[level + 1]  = &node->m_childs[lvl_index[level]];
                lvl_index[level]++;
                level++;
            }
        }
        fclose(f);
        return true;
    }
private:
    aist_mem_T m_mem;
    xml_node*  m_root;
    size_t     m_root_count;

    const char* m_xml_header;
    size_t      m_xml_header_len;
};

