/*
 * Copyright (c) 2010-2012 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "thingtypemanager.h"
#include "spritemanager.h"
#include "thing.h"
#include "thingtypedat.h"
#include "thingtypeotb.h"

#include <framework/core/resourcemanager.h>
#include <framework/core/filestream.h>
#include <framework/core/binarytree.h>
#include <framework/xml/tinyxml.h>

ThingTypeManager g_things;

void ThingTypeManager::init()
{
    m_nullDatType = ThingTypeDatPtr(new ThingTypeDat);
    m_nullOtbType = ThingTypeOtbPtr(new ThingTypeOtb);
    m_datSignature = 0;
    m_otbMinorVersion = 0;
    m_otbMajorVersion = 0;
    m_datLoaded = false;
    m_xmlLoaded = false;
    m_otbLoaded = false;
}

void ThingTypeManager::terminate()
{
    for(int i = 0; i < DatLastCategory; ++i)
        m_datTypes[i].clear();
    m_otbTypes.clear();
    m_nullDatType = nullptr;
    m_nullOtbType = nullptr;
}

bool ThingTypeManager::loadDat(const std::string& file)
{
    try {
        FileStreamPtr fin = g_resources.openFile(file);
        if(!fin)
            stdext::throw_exception("unable to open file");

        m_datSignature = fin->getU32();

        int numThings[DatLastCategory];
        for(int category = 0; category < DatLastCategory; ++category) {
            int count = fin->getU16() + 1;
            m_datTypes[category].resize(count, m_nullDatType);
        }

        for(int category = 0; category < DatLastCategory; ++category) {
            uint16 firstId = 1;
            if(category == DatItemCategory)
                firstId = 100;
            for(uint16 id = firstId; id < m_datTypes[category].size(); ++id) {
                ThingTypeDatPtr type(new ThingTypeDat);
                type->unserialize(id, (DatCategory)category, fin);
                m_datTypes[category][id] = type;
            }
        }

        m_datLoaded = true;
        return true;
    } catch(stdext::exception& e) {
        g_logger.error(stdext::format("Failed to read dat '%s': %s'", file, e.what()));
        return false;
    }
}

void ThingTypeManager::loadOtb(const std::string& file)
{
    FileStreamPtr fin = g_resources.openFile(file);

    uint signature = fin->getU32();
    if(signature != 0)
        stdext::throw_exception("invalid otb file");

    BinaryTreePtr root = fin->getBinaryTree();

    signature = root->getU32();
    if(signature != 0)
        stdext::throw_exception("invalid otb file");

    root->getU32(); // flags

    m_otbMajorVersion = root->getU32();
    m_otbMinorVersion = root->getU32();
    root->getU32(); // build number
    root->skip(128); // description

    m_otbTypes.resize(root->getChildren().size(), m_nullOtbType);
    for(const BinaryTreePtr& node : root->getChildren()) {
        ThingTypeOtbPtr otbType(new ThingTypeOtb);
        otbType->unserialize(node);
        addOtbType(otbType);
    }

    m_otbLoaded = true;
}

void ThingTypeManager::loadXml(const std::string& file)
{
    TiXmlDocument doc(file.c_str());
    if(!doc.LoadFile())
        stdext::throw_exception(stdext::format("failed to load xml '%s'", file));

    TiXmlElement* root = doc.FirstChildElement();
    if(!root || root->ValueTStr() != "items")
        stdext::throw_exception("invalid root tag name");

    ThingTypeOtbPtr otbType = nullptr;
    for (TiXmlElement *element = root->FirstChildElement(); element; element = element->NextSiblingElement()) {
        if(element->ValueTStr() != "item")
            continue;

        std::string name = element->Attribute("id");
        if(name.empty())
            continue;

        uint16 id = stdext::unsafe_cast<uint16>(element->Attribute("id"));
        if(!(otbType = getOtbType(id))) {
            // try reading fromId toId
            uint16 from = stdext::unsafe_cast<uint16>(element->Attribute("fromId"));
            uint16 to = stdext::unsafe_cast<uint16>(element->Attribute("toid"));

            for (uint16 __id = from; __id < to; ++__id) {
                if(!(otbType = getOtbType(__id)))
                    continue;

                otbType->setHasRange();
                otbType->setFromServerId(from);
                otbType->setToServerId(to);
                break;
            }

            // perform last check
            if(!otbType) {
                stdext::throw_exception(stdext::format("failed to find item with server id %d - tried reading fromid to id",
                                                       id));
            }
        }

        for (TiXmlElement *attr = element->FirstChildElement(); attr; attr = attr->NextSiblingElement()) {
            if(attr->ValueTStr() != "attribute")
                continue;

            otbType->unserializeXML(attr);
        }
    }

    doc.Clear();
    m_xmlLoaded = true;
}

void ThingTypeManager::addOtbType(const ThingTypeOtbPtr& otbType)
{
    uint16 id = otbType->getServerId();
    if(m_otbTypes.size() <= id)
        m_otbTypes.resize(id+1, m_nullOtbType);
    m_otbTypes[id] = otbType;
}

const ThingTypeOtbPtr& ThingTypeManager::findOtbForClientId(uint16 id)
{
    if(m_otbTypes.empty())
        return m_nullOtbType;

    for(const ThingTypeOtbPtr& otbType : m_otbTypes) {
        if(otbType->getClientId() == id)
            return otbType;
    }

    return m_nullOtbType;
}

const ThingTypeDatPtr& ThingTypeManager::getDatType(uint16 id, DatCategory category)
{
    if(category >= DatLastCategory || id >= m_datTypes[category].size()) {
        g_logger.error(stdext::format("invalid thing type client id %d in category %d", id, category));
        return m_nullDatType;
    }
    return m_datTypes[category][id];
}

const ThingTypeOtbPtr& ThingTypeManager::getOtbType(uint16 id)
{
    if(id >= m_otbTypes.size()) {
        g_logger.error(stdext::format("invalid thing type server id %d", id));
        return m_nullOtbType;
    }
    return m_otbTypes[id];
}
