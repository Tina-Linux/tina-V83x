/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : CdxId3Parser.c
* Description :
* History :
*   Author  : Khan <chengkan@allwinnertech.com>
*   Date    : 2014/12/08
*   Comment : 创建初始版本，实现 ID3_tag 的解析功能
*/

#include <CdxTypes.h>
#include <CdxParser.h>
#include <CdxStream.h>
#include <CdxMemory.h>
#include <CdxId3Parser.h>

#define IMAGE_MIME_TYPE_BMP     "image/bmp"
#define IMAGE_MIME_TYPE_JPEG    "image/jpeg"
#define IMAGE_MIME_TYPE_PNG        "image/png"

static const language_coding_t ISO_639_2_Code[545] = {
    //ISO_639_2_Code                        //English name of Language
    {"aar",-1},                          //Afar
    {"abk",-1},                    //Abkhazian
    {"ace",-1},                    //Achinese
    {"ach",-1},                    //Acoli
    {"ada",-1},                    //Adangme
    {"ady",-1},                    //Adyghe; Adygei
    {"afa",-1},                    //Afro-Asiatic languages
    {"afh",-1},                    //Afrihili
    {"afr",-1},                    //Afrikaans
    {"ain",-1},                    //Ainu
    {"aka",-1},                    //Akan
    {"akk",-1},                    //Akkadian
    {"alb",-1},                    //Albanian
    {"sqi",-1},                    //
    {"ale",-1},                    //Aleut
    {"alg",-1},                    //Algonquian languages
    {"alt",-1},                    //Southern Altai
    {"amh",-1},                    //Amharic
    {"ang",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //English, Old (ca.450-1100)
    {"anp",-1},                    //Angika
    {"apa",-1},                    //Apache languages
    {"ara",-1},                    //Arabic
    {"arc",-1},                    //Official Aramaic (700-300 BCE); Imperial Aramaic (700-300 BCE)
    {"arg",-1},                    //Aragonese
    {"arm",-1},                    //Armenian
    {"hye",-1},                    //
    {"arn",-1},                    //Mapudungun; Mapuche
    {"arp",-1},                    //Arapaho
    {"art",-1},                    //Artificial languages
    {"arw",-1},                    //Arawak
    {"asm",-1},                    //Assamese
    {"ast",-1},                    //Asturian; Bable; Leonese; Asturleonese
    {"ath",-1},                    //Athapascan languages
    {"aus",-1},                    //Australian languages
    {"ava",-1},                    //Avaric
    {"ave",-1},                    //Avestan
    {"awa",-1},                    //Awadhi
    {"aym",-1},                    //Aymara
    {"aze",-1},                    //Azerbaijani
    {"bad",-1},                    //Banda languages
    {"bai",-1},                    //Bamileke languages
    {"bak",-1},                    //Bashkir
    {"bal",-1},                    //Baluchi
    {"bam",-1},                    //Bambara
    {"ban",-1},                    //Balinese
    {"baq",-1},                    //Basque
    {"eus",-1},                    //
    {"bas",-1},                    //Basa
    {"bat",-1},                    //Baltic languages
    {"bej",-1},                    //Beja; Bedawiyet
    {"bel",A_AUDIO_FONTTYPE_ISOIEC8859_5},                    //Belarusian
    {"bem",-1},                    //Bemba
    {"ben",-1},                    //Bengali
    {"ber",-1},                    //Berber languages
    {"bho",-1},                    //Bhojpuri
    {"bih",-1},                    //Bihari languages
    {"bik",-1},                    //Bikol
    {"bin",-1},                    //Bini; Edo
    {"bis",-1},                    //Bislama
    {"bla",-1},                    //Siksika
    {"bnt",-1},                    //Bantu languages
    {"tib",-1},                    //Tibetan
    {"bod",-1},                    //
    {"bos",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Bosnian
    {"bra",-1},                    //Braj
    {"bre",-1},                    //Breton
    {"btk",-1},                    //Batak languages
    {"bua",-1},                    //Buriat
    {"bug",-1},                    //Buginese
    {"bul",A_AUDIO_FONTTYPE_ISOIEC8859_5},                    //Bulgarian
    {"bur",-1},                    //Burmese
    {"mya",-1},                    //
    {"byn",-1},                    //Blin; Bilin
    {"cad",-1},                    //Caddo
    {"cai",-1},                    //Central American Indian languages
    {"car",-1},                    //Galibi Carib
    {"cat",-1},                    //Catalan; Valencian
    {"cau",-1},                    //Caucasian languages
    {"ceb",-1},                    //Cebuano
    {"cel",-1},                    //Celtic languages
    {"cze",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Czech
    {"ces",-1},                    //
    {"cha",-1},                    //Chamorro
    {"chb",-1},                    //Chibcha
    {"che",-1},                    //Chechen
    {"chg",-1},                    //Chagatai
    {"chi",-1},                    //Chinese
    {"zho",-1},                    //
    {"chk",-1},                    //Chuukese
    {"chm",-1},                    //Mari
    {"chn",-1},                    //Chinook jargon
    {"cho",-1},                    //Choctaw
    {"chp",-1},                    //Chipewyan; Dene Suline
    {"chr",-1},                    //Cherokee
    {"chu",-1},                    //Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian;
                                   //Old Church Slavonic

    {"chv",-1},                    //Chuvash
    {"chy",-1},                    //Cheyenne
    {"cmc",-1},                    //Chamic languages
    {"cop",-1},                    //Coptic
    {"cor",-1},                    //Cornish
    {"cos",-1},                    //Corsican
    {"cpe",-1},                    //Creoles and pidgins, English based
    {"cpf",-1},                    //Creoles and pidgins, French-based
    {"cpp",-1},                    //Creoles and pidgins, Portuguese-based
    {"cre",-1},                    //Cree
    {"crh",-1},                    //Crimean Tatar; Crimean Turkish
    {"crp",-1},                    //Creoles and pidgins
    {"csb",-1},                    //Kashubian
    {"cus",-1},                    //Cushitic languages
    {"wel",-1},                    //Welsh
    {"cym",-1},                    //
    {"cze",-1},                    //Czech
    {"ces",-1},                    //
    {"dak",-1},                    //Dakota
    {"dan",-1},                    //Danish
    {"dar",-1},                    //Dargwa
    {"day",-1},                    //Land Dayak languages
    {"del",-1},                    //Delaware
    {"den",-1},                    //Slave (Athapascan)
    {"ger",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //German
    {"deu",-1},                    //
    {"dgr",-1},                    //Dogrib
    {"din",-1},                    //Dinka
    {"div",-1},                    //Divehi; Dhivehi; Maldivian
    {"doi",-1},                    //Dogri
    {"dra",-1},                    //Dravidian languages
    {"dsb",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Lower Sorbian
    {"dua",-1},                    //Duala
    {"dum",-1},                    //Dutch, Middle (ca.1050-1350)
    {"dut",-1},                    //Dutch; Flemish
    {"nld",-1},                    //
    {"dyu",-1},                    //Dyula
    {"dzo",-1},                    //Dzongkha
    {"efi",-1},                    //Efik
    {"egy",-1},                    //Egyptian (Ancient)
    {"eka",-1},                    //Ekajuk
    {"gre",-1},                    //Greek, Modern (1453-)
    {"ell",-1},                    //
    {"elx",-1},                    //Elamite
    {"eng",-1},                    //English
    {"enm",-1},                    //English, Middle (1100-1500)
    {"epo",A_AUDIO_FONTTYPE_ISOIEC8859_3},                    //Esperanto
    {"est",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Estonian
    {"baq",-1},                    //Basque
    {"eus",-1},                    //
    {"ewe",-1},                    //Ewe
    {"ewo",-1},                    //Ewondo
    {"fan",-1},                    //Fang
    {"fao",-1},                    //Faroese
    {"per",-1},                    //Persian
    {"fas",-1},                    //
    {"fat",-1},                    //Fanti
    {"fij",-1},                    //Fijian
    {"fil",-1},                    //Filipino; Pilipino
    {"fin",-1},                    //Finnish
    {"fiu",-1},                    //Finno-Ugrian languages
    {"fon",-1},                    //Fon
    {"fre",-1},                    //French
    {"fra",-1},                    //
    {"fre",-1},                    //French
    {"fra",-1},                    //
    {"frm",-1},                    //French, Middle (ca.1400-1600)
    {"fro",-1},                    //French, Old (842-ca.1400)
    {"frr",-1},                    //Northern Frisian
    {"frs",-1},                    //Eastern Frisian
    {"fry",-1},                    //Western Frisian
    {"ful",-1},                    //Fulah
    {"fur",-1},                    //Friulian
    {"gaa",-1},                    //Ga
    {"gay",-1},                    //Gayo
    {"gba",-1},                    //Gbaya
    {"gem",-1},                    //Germanic languages
    {"geo",-1},                    //Georgian
    {"kat",-1},                    //
    {"ger",-1},                    //German
    {"deu",-1},                    //
    {"gez",-1},                    //Geez
    {"gil",-1},                    //Gilbertese
    {"gla",-1},                    //Gaelic; Scottish Gaelic
    {"gle",-1},                    //Irish
    {"glg",-1},                    //Galician
    {"glv",-1},                    //Manx
    {"gmh",-1},                    //German, Middle High (ca.1050-1500)
    {"goh",-1},                    //German, Old High (ca.750-1050)
    {"gon",-1},                    //Gondi
    {"gor",-1},                    //Gorontalo
    {"got",-1},                    //Gothic
    {"grb",-1},                    //Grebo
    {"grc",-1},                    //Greek, Ancient (to 1453)
    {"gre",-1},                    //Greek, Modern (1453-)
    {"ell",-1},                    //
    {"grn",-1},                    //Guarani
    {"gsw",-1},                    //Swiss German; Alemannic; Alsatian
    {"guj",-1},                    //Gujarati
    {"gwi",-1},                    //Gwich'in
    {"hai",-1},                    //Haida
    {"hat",-1},                    //Haitian; Haitian Creole
    {"hau",-1},                    //Hausa
    {"haw",-1},                    //Hawaiian
    {"heb",-1},                    //Hebrew
    {"her",-1},                    //Herero
    {"hil",-1},                    //Hiligaynon
    {"him",-1},                    //Himachali languages; Western Pahari languages
    {"hin",-1},                    //Hindi
    {"hit",-1},                    //Hittite
    {"hmn",-1},                    //Hmong; Mong
    {"hmo",-1},                    //Hiri Motu
    {"hrv",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Croatian
    {"hsb",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Upper Sorbian
    {"hun",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Hungarian
    {"hup",-1},                    //Hupa
    {"arm",-1},                    //Armenian
    {"hye",-1},                    //
    {"iba",-1},                    //Iban
    {"ibo",-1},                    //Igbo
    {"ice",-1},                    //Icelandic
    {"isl",-1},                    //
    {"ido",-1},                    //Ido
    {"iii",-1},                    //Sichuan Yi; Nuosu
    {"ijo",-1},                    //Ijo languages
    {"iku",-1},                    //Inuktitut
    {"ile",-1},                    //Interlingue; Occidental
    {"ilo",-1},                    //Iloko
    {"ina",-1},                    //Interlingua (International Auxiliary Language Association)
    {"inc",-1},                    //Indic languages
    {"ind",-1},                    //Indonesian
    {"ine",-1},                    //Indo-European languages
    {"inh",-1},                    //Ingush
    {"ipk",-1},                    //Inupiaq
    {"ira",-1},                    //Iranian languages
    {"iro",-1},                    //Iroquoian languages
    {"ice",-1},                    //Icelandic
    {"isl",-1},                    //
    {"ita",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //Italian
    {"jav",-1},                    //Javanese
    {"jbo",-1},                    //Lojban
    {"jpn",-1},                    //Japanese
    {"jpr",-1},                    //Judeo-Persian
    {"jrb",-1},                    //Judeo-Arabic
    {"kaa",-1},                    //Kara-Kalpak
    {"kab",-1},                    //Kabyle
    {"kac",-1},                    //Kachin; Jingpho
    {"kal",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Kalaallisut; Greenlandic
    {"kam",-1},                    //Kamba
    {"kan",-1},                    //Kannada
    {"kar",-1},                    //Karen languages
    {"kas",-1},                    //Kashmiri
    {"geo",-1},                    //Georgian
    {"kat",-1},                    //
    {"kau",-1},                    //Kanuri
    {"kaw",-1},                    //Kawi
    {"kaz",-1},                    //Kazakh
    {"kbd",-1},                    //Kabardian
    {"kha",-1},                    //Khasi
    {"khi",-1},                    //Khoisan languages
    {"khm",-1},                    //Central Khmer
    {"kho",-1},                    //Khotanese; Sakan
    {"kik",-1},                    //Kikuyu; Gikuyu
    {"kin",-1},                    //Kinyarwanda
    {"kir",-1},                    //Kirghiz; Kyrgyz
    {"kmb",-1},                    //Kimbundu
    {"kok",-1},                    //Konkani
    {"kom",-1},                    //Komi
    {"kon",-1},                    //Kongo
    {"kor",-1},                    //Korean
    {"kos",-1},                    //Kosraean
    {"kpe",-1},                    //Kpelle
    {"krc",-1},                    //Karachay-Balkar
    {"krl",-1},                    //Karelian
    {"kro",-1},                    //Kru languages
    {"kru",-1},                    //Kurukh
    {"kua",-1},                    //Kuanyama; Kwanyama
    {"kum",-1},                    //Kumyk
    {"kur",-1},                    //Kurdish
    {"kut",-1},                    //Kutenai
    {"lad",-1},                    //Ladino
    {"lah",-1},                    //Lahnda
    {"lam",-1},                    //Lamba
    {"lao",-1},                    //Lao
    {"lat",-1},                    //Latin
    {"lav",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Latvian
    {"lez",-1},                    //Lezghian
    {"lim",-1},                    //Limburgan; Limburger; Limburgish
    {"lin",-1},                    //Lingala
    {"lit",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Lithuanian
    {"lol",-1},                    //Mongo
    {"loz",-1},                    //Lozi
    {"ltz",-1},                    //Luxembourgish; Letzeburgesch
    {"lua",-1},                    //Luba-Lulua
    {"lub",-1},                    //Luba-Katanga
    {"lug",-1},                    //Ganda
    {"lui",-1},                    //Luiseno
    {"lun",-1},                    //Lunda
    {"luo",-1},                    //Luo (Kenya and Tanzania)
    {"lus",-1},                    //Lushai
    {"mac",-1},                    //Macedonian
    {"mkd",-1},                    //
    {"mad",-1},                    //Madurese
    {"mag",-1},                    //Magahi
    {"mah",-1},                    //Marshallese
    {"mai",-1},                    //Maithili
    {"mak",-1},                    //Makasar
    {"mal",-1},                    //Malayalam
    {"man",-1},                    //Mandingo
    {"mao",-1},                    //Maori
    {"mri",-1},                    //
    {"map",-1},                    //Austronesian languages
    {"mar",-1},                    //Marathi
    {"mas",-1},                    //Masai
    {"may",-1},                    //Malay
    {"msa",-1},                    //
    {"mdf",-1},                    //Moksha
    {"mdr",-1},                    //Mandar
    {"men",-1},                    //Mende
    {"mga",-1},                    //Irish, Middle (900-1200)
    {"mic",-1},                    //Mi'kmaq; Micmac
    {"min",-1},                    //Minangkabau
    {"mis",-1},                    //Uncoded languages
    {"mac",-1},                    //Macedonian
    {"mkd",-1},                    //
    {"mkh",-1},                    //Mon-Khmer languages
    {"mlg",-1},                    //Malagasy
    {"mlt",A_AUDIO_FONTTYPE_ISOIEC8859_3},                    //Maltese
    {"mnc",-1},                    //Manchu
    {"mni",-1},                    //Manipuri
    {"mno",-1},                    //Manobo languages
    {"moh",-1},                    //Mohawk
    {"mon",-1},                    //Mongolian
    {"mos",-1},                    //Mossi
    {"mao",-1},                    //Maori
    {"mri",-1},                    //
    {"may",-1},                    //Malay
    {"msa",-1},                    //
    {"mul",-1},                    //Multiple languages
    {"mun",-1},                    //Munda languages
    {"mus",-1},                    //Creek
    {"mwl",-1},                    //Mirandese
    {"mwr",-1},                    //Marwari
    {"bur",-1},                    //Burmese
    {"mya",-1},                    //
    {"myn",-1},                    //Mayan languages
    {"myv",-1},                    //Erzya
    {"nah",-1},                    //Nahuatl languages
    {"nai",-1},                    //North American Indian languages
    {"nap",-1},                    //Neapolitan
    {"nau",-1},                    //Nauru
    {"nav",-1},                    //Navajo; Navaho
    {"nbl",-1},                    //Ndebele, South; South Ndebele
    {"nde",-1},                    //Ndebele, North; North Ndebele
    {"ndo",-1},                    //Ndonga
    {"nds",-1},                    //Low German; Low Saxon; German, Low; Saxon, Low
    {"nep",-1},                    //Nepali
    {"new",-1},                    //Nepal Bhasa; Newari
    {"nia",-1},                    //Nias
    {"nic",-1},                    //Niger-Kordofanian languages
    {"niu",-1},                    //Niuean
    {"dut",-1},                    //Dutch; Flemish
    {"nld",-1},                    //
    {"nno",-1},                    //Norwegian Nynorsk; Nynorsk, Norwegian
    {"nob",-1},                    //Bokm?l, Norwegian; Norwegian Bokm?l
    {"nog",-1},                    //Nogai
    {"non",-1},                    //Norse, Old
    {"nor",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //Norwegian
    {"nqo",-1},                    //N'Ko
    {"nso",-1},                    //Pedi; Sepedi; Northern Sotho
    {"nub",-1},                    //Nubian languages
    {"nwc",-1},                    //Classical Newari; Old Newari; Classical Nepal Bhasa
    {"nya",-1},                    //Chichewa; Chewa; Nyanja
    {"nym",-1},                    //Nyamwezi
    {"nyn",-1},                    //Nyankole
    {"nyo",-1},                    //Nyoro
    {"nzi",-1},                    //Nzima
    {"oci",-1},                    //Occitan (post 1500)
    {"oji",-1},                    //Ojibwa
    {"ori",-1},                    //Oriya
    {"orm",-1},                    //Oromo
    {"osa",-1},                    //Osage
    {"oss",-1},                    //Ossetian; Ossetic
    {"ota",-1},                    //Turkish, Ottoman (1500-1928)
    {"oto",-1},                    //Otomian languages
    {"paa",-1},                    //Papuan languages
    {"pag",-1},                    //Pangasinan
    {"pal",-1},                    //Pahlavi
    {"pam",-1},                    //Pampanga; Kapampangan
    {"pan",-1},                    //Panjabi; Punjabi
    {"pap",-1},                    //Papiamento
    {"pau",-1},                    //Palauan
    {"peo",-1},                    //Persian, Old (ca.600-400 B.C.)
    {"per",-1},                    //Persian
    {"fas",-1},                    //
    {"phi",-1},                    //Philippine languages
    {"phn",-1},                    //Phoenician
    {"pli",-1},                    //Pali
    {"pol",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Polish
    {"pon",-1},                    //Pohnpeian
    {"por",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //Portuguese
    {"pra",-1},                    //Prakrit languages
    {"pro",-1},                    //Proven?al, Old (to 1500);Occitan, Old (to 1500)
    {"pus",-1},                    //Pushto; Pashto
    {"qaa",-1},                    //Reserved for local use
    {"que",-1},                    //Quechua
    {"raj",-1},                    //Rajasthani
    {"rap",-1},                    //Rapanui
    {"rar",-1},                    //Rarotongan; Cook Islands Maori
    {"roa",-1},                    //Romance languages
    {"roh",-1},                    //Romansh
    {"rom",-1},                    //Romany
    {"rum",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Romanian; Moldavian; Moldovan
    {"ron",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //
    {"rum",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Romanian; Moldavian; Moldovan
    {"ron",-1},                    //
    {"run",-1},                    //Rundi
    {"rup",-1},                    //Aromanian; Arumanian; Macedo-Romanian
    {"rus",A_AUDIO_FONTTYPE_WINDOWS_1251},                    //Russian
    {"sad",-1},                    //Sandawe
    {"sag",-1},                    //Sango
    {"sah",-1},                    //Yakut
    {"sai",-1},                    //South American Indian languages
    {"sal",-1},                    //Salishan languages
    {"sam",-1},                    //Samaritan Aramaic
    {"san",-1},                    //Sanskrit
    {"sas",-1},                    //Sasak
    {"sat",-1},                    //Santali
    {"scn",-1},                    //Sicilian
    {"sco",-1},                    //Scots
    {"sel",-1},                    //Selkup
    {"sem",-1},                    //Semitic languages
    {"sga",-1},                    //Irish, Old (to 900)
    {"sgn",-1},                    //Sign Languages
    {"shn",-1},                    //Shan
    {"sid",-1},                    //Sidamo
    {"sin",-1},                    //Sinhala; Sinhalese
    {"sio",-1},                    //Siouan languages
    {"sit",-1},                    //Sino-Tibetan languages
    {"sla",-1},                    //Slavic languages
    {"slo",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Slovak
    {"slk",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //
    {"slo",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Slovak
    {"slk",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //
    {"slv",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Slovenian //maybe slovene????
    {"sma",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Southern Sami
    {"sme",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Northern Sami
    {"smi",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Sami languages
    {"smj",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Lule Sami
    {"smn",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Inari Sami
    {"smo",-1},                    //Samoan
    {"sms",A_AUDIO_FONTTYPE_ISOIEC8859_4},                    //Skolt Sami
    {"sna",-1},                    //Shona
    {"snd",-1},                    //Sindhi
    {"snk",-1},                    //Soninke
    {"sog",-1},                    //Sogdian
    {"som",-1},                    //Somali
    {"son",-1},                    //Songhai languages
    {"sot",-1},                    //Sotho, Southern
    {"spa",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //Spanish; Castilian
    {"alb",-1},                    //Albanian
    {"sqi",-1},                    //
    {"srd",-1},                    //Sardinian
    {"srn",-1},                    //Sranan Tongo
    {"srp",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Serbian
    {"srr",-1},                    //Serer
    {"ssa",-1},                    //Nilo-Saharan languages
    {"ssw",-1},                    //Swati
    {"suk",-1},                    //Sukuma
    {"sun",-1},                    //Sundanese
    {"sus",-1},                    //Susu
    {"sux",-1},                    //Sumerian
    {"swa",-1},                    //Swahili
    {"swe",A_AUDIO_FONTTYPE_ISOIEC8859_1},                    //Swedish
    {"syc",-1},                    //Classical Syriac
    {"syr",-1},                    //Syriac
    {"tah",-1},                    //Tahitian
    {"tai",-1},                    //Tai languages
    {"tam",-1},                    //Tamil
    {"tat",-1},                    //Tatar
    {"tel",-1},                    //Telugu
    {"tem",-1},                    //Timne
    {"ter",-1},                    //Tereno
    {"tet",-1},                    //Tetum
    {"tgk",-1},                    //Tajik
    {"tgl",-1},                    //Tagalog
    {"tha",-1},                    //Thai
    {"tib",-1},                    //Tibetan
    {"bod",-1},                    //
    {"tig",-1},                    //Tigre
    {"tir",-1},                    //Tigrinya
    {"tiv",-1},                    //Tiv
    {"tkl",-1},                    //Tokelau
    {"tlh",-1},                    //Klingon; tlhIngan-Hol
    {"tli",-1},                    //Tlingit
    {"tmh",-1},                    //Tamashek
    {"tog",-1},                    //Tonga (Nyasa)
    {"ton",-1},                    //Tonga (Tonga Islands)
    {"tpi",-1},                    //Tok Pisin
    {"tsi",-1},                    //Tsimshian
    {"tsn",-1},                    //Tswana
    {"tso",-1},                    //Tsonga
    {"tuk",-1},                    //Turkmen
    {"tum",-1},                    //Tumbuka
    {"tup",-1},                    //Tupi languages
    {"tur",A_AUDIO_FONTTYPE_ISOIEC8859_3},                    //Turkish
    {"tut",-1},                    //Altaic languages
    {"tvl",-1},                    //Tuvalu
    {"twi",-1},                    //Twi
    {"tyv",-1},                    //Tuvinian
    {"udm",-1},                    //Udmurt
    {"uga",-1},                    //Ugaritic
    {"uig",-1},                    //Uighur; Uyghur
    {"ukr",-1},                    //Ukrainian
    {"umb",-1},                    //Umbundu
    {"und",-1},                    //Undetermined
    {"urd",-1},                    //Urdu
    {"uzb",-1},                    //Uzbek
    {"vai",-1},                    //Vai
    {"ven",-1},                    //Venda
    {"vie",-1},                    //Vietnamese
    {"vol",-1},                    //Volapük
    {"vot",-1},                    //Votic
    {"wak",-1},                    //Wakashan languages
    {"wal",-1},                    //Wolaitta; Wolaytta
    {"war",-1},                    //Waray
    {"was",-1},                    //Washo
    {"wel",-1},                    //Welsh
    {"cym",-1},                    //
    {"wen",A_AUDIO_FONTTYPE_ISOIEC8859_2},                    //Sorbian languages ???geuss
    {"wln",-1},                    //Walloon
    {"wol",-1},                    //Wolof
    {"xal",-1},                    //Kalmyk; Oirat
    {"xho",-1},                    //Xhosa
    {"yao",-1},                    //Yao
    {"yap",-1},                    //Yapese
    {"yid",-1},                    //Yiddish
    {"yor",-1},                    //Yoruba
    {"ypk",-1},                    //Yupik languages
    {"zap",-1},                    //Zapotec
    {"zbl",-1},                    //Blissymbols; Blissymbolics; Bliss
    {"zen",-1},                    //Zenaga
    {"zha",-1},                    //Zhuang; Chuang
    {"chi",-1},                    //Chinese
    {"zho",-1},                    //
    {"znd",-1},                    //Zande languages
    {"zul",-1},                    //Zulu
    {"zun",-1},                    //Zuni
    {"zxx",-1},                    //No linguistic content; Not applicable
    {"zza",-1},                    //Zaza; Dimili; Dimli; Kirdki; Kirmanjki; Zazaki
};

static const char ID3V1GENRE[148+2][22] =
{
    "Blues",
    "ClassicRock",
    "Country",
    "Dance",
    "Disco",
    "Funk",
    "Grunge",
    "Hip-Hop",
    "Jazz",
    "Metal",
    "NewAge",
    "Oldies",
    "Other",
    "Pop",
    "R&B",
    "Rap",
    "Reggae",
    "Rock",
    "Techno",
    "Industrial",
    "Alternative",
    "Ska",
    "DeathMetal",
    "Pranks",
    "Soundtrack",
    "Euro-Techno",
    "Ambient",
    "Trip-Hop",
    "Vocal",
    "Jazz+Funk",
    "Fusion",
    "Trance",
    "Classical",
    "Instrumental",
    "Acid",
    "House",
    "Game",
    "SoundClip",
    "Gospel",
    "Noise",
    "AlternRock",
    "Bass",
    "Soul",
    "Punk",
    "Space",
    "Meditative",
    "InstrumentalPop",
    "InstrumentalRock",
    "Ethnic",
    "Gothic",
    "Darkwave",
    "Techno-Industrial",
    "Electronic",
    "Pop-Folk",
    "Eurodance",
    "Dream",
    "SouthernRock",
    "Comedy",
    "Cult",
    "Gangsta",
    "Top40",
    "ChristianRap",
    "Pop/Funk",
    "Jungle",
    "NativeAmerican",
    "Cabaret",
    "NewWave",
    "Psychadelic",
    "Rave",
    "Showtunes",
    "Trailer",
    "Lo-Fi",
    "Tribal",
    "AcidPunk",
    "AcidJazz",
    "Polka",
    "Retro",
    "Musical",
    "Rock&Roll",
    "HardRock",
/* Extended genres */
    "Folk",
    "Folk-Rock",
    "NationalFolk",
    "Swing",
    "FastFusion",
    "Bebob",
    "Latin",
    "Revival",
    "Celtic",
    "Bluegrass",
    "Avantgarde",
    "GothicRock",
    "ProgessiveRock",
    "PsychedelicRock",
    "SymphonicRock",
    "SlowRock",
    "BigBand",
    "Chorus",
    "EasyListening",
    "Acoustic",
    "Humour",
    "Speech",
    "Chanson",
    "Opera",
    "ChamberMusic",
    "Sonata",
    "Symphony",
    "BootyBass",
    "Primus",
    "PornGroove",
    "Satire",
    "SlowJam",
    "Club",
    "Tango",
    "Samba",
    "Folklore",
    "Ballad",
    "PowerBallad",
    "RhythmicSoul",
    "Freestyle",
    "Duet",
    "PunkRock",
    "DrumSolo",
    "Acapella",
    "Euro-House",
    "DanceHall",
    "Goa",
    "Drum&Bass",
    "Club-House",
    "Hardcore",
    "Terror",
    "Indie",
    "BritPop",
    "Negerpunk",
    "PolskPunk",
    "Beat",
    "ChristianGangstaRap",
    "HeavyMetal",
    "BlackMetal",
    "Crossover",
    "ContemporaryChristian",
    "ChristianRock",
    "Merengue",
    "Salsa",
    "TrashMetal",
    "Anime",
    "JPop",
    "Synthpop",
    "Remix",
    "Cover"
};

unsigned short windows1251_unicode[128] = {
    0x0402,    0x0403,    0x201A,    0x0453,    0x201E,    0x2026,    0x2020,    0x2021,
    0x20AC,    0x2030,    0x0409,    0x2039,    0x040A,    0x040C,    0x040B,    0x040F,
    0x0452,    0x2018,    0x2019,    0x201C,    0x201D,    0x2022,    0x2013,    0x2014,
    0x0020,/*undefined*/  0x2122,    0x0459,    0x203A,    0x045A,    0x045C,    0x045B,    0x045F,
    0x00A0,    0x040E,    0x045E,    0x0408,    0x00A4,    0x0490,    0x00A6,    0x00A7,
    0x0401,    0x00A9,    0x0404,    0x00AB,    0x00AC,    0x00AD,    0x00AE,    0x0407,
    0x00B0,    0x00B1,    0x0406,    0x0456,    0x0491,    0x00B5,    0x00B6,    0x00B7,
    0x0451,    0x2116,    0x0454,    0x00BB,    0x0458,    0x0405,    0x0455,    0x0457,
    0x0410,    0x0411,    0x0412,    0x0413,    0x0414,    0x0415,    0x0416,    0x0417,
    0x0418,    0x0419,    0x041A,    0x041B,    0x041C,    0x041D,    0x041E,    0x041F,
    0x0420,    0x0421,    0x0422,    0x0423,    0x0424,    0x0425,    0x0426,    0x0427,
    0x0428,    0x0429,    0x042A,    0x042B,    0x042C,    0x042D,    0x042E,    0x042F,
    0x0430,    0x0431,    0x0432,    0x0433,    0x0434,    0x0435,    0x0436,    0x0437,
    0x0438,    0x0439,    0x043A,    0x043B,    0x043C,    0x043D,    0x043E,    0x043F,
    0x0440,    0x0441,    0x0442,    0x0443,    0x0444,    0x0445,    0x0446,    0x0447,
    0x0448,    0x0449,    0x044A,    0x044B,    0x044C,    0x044D,    0x044E,    0x044F
};

static const unsigned    int ID3UniqueTag[2][ID3TAGNUM]=
{
    { //ID3V2.3 2.4
        0x54495432,//TIT2
        0x54504531,//TPE1
        0x54414C42,//TALB
        0x54594552,//TYER
        0x54434f4e,//TCON
        0x41504943,//APIC
        0x434f4d4d,//COMM
        0x544c414e//TLAN
    },
    { //ID3V2.1
        0x545432,//TT2
        0x545031,//TP1
        0x54414C,//TAL
        0x545945,//TYE
        0x54434f,//TCO
        0x415049,//API
        0x434f4d,//COM
        0x544c41//TLA
    }
};

static uint16_t U16_AT(const uint8_t *ptr) {
    return ptr[0] << 8 | ptr[1];
}

static uint32_t U32_AT(const uint8_t *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static uint32_t U24_AT(const uint8_t *ptr) {
    return ptr[0] << 16 | ptr[1] << 8 | ptr[2];
}

#define AV_RB32(x)  ((((const unsigned char*)(x))[0] << 24) | \
    (((const unsigned char*)(x))[1] << 16) | \
    (((const unsigned char*)(x))[2] <<  8) | \
                      ((const unsigned char*)(x))[3])

cdx_uint32    ID3PsrGetBsInByte(cdx_int32     ByteLen,Id3ParserImplS *id3,cdx_int32 move_ptr);

cdx_bool ID3Switch2SyncsafeInteger(cdx_int32 *x) {
    cdx_int32 i, tmp;
    cdx_uint8 encoded[4] = {0};
    tmp = *x;
    *x = 0;
    encoded[0] = ((tmp>>24)&0xff);
    encoded[1] = ((tmp>>16)&0xff);
    encoded[2] = ((tmp>>8) &0xff);
    encoded[3] = ( tmp     &0xff);
    for (i = 0; i < 4; ++i)
    {
        if (encoded[i] & 0x80)
        {
            return CDX_FALSE;
        }
        *x = ((*x) << 7) | encoded[i];
    }
    return CDX_TRUE;
}

cdx_int32 ID3PsrGETID3Len(cdx_uint8* ptr,cdx_int32 len)
{
    cdx_int32 Id3v2len = 0;
    if(len<10)
    {
        return 0;
    }
    if((ptr[0]==0x49)&&(ptr[1]==0x44)&&(ptr[2]==0x33))
    {
        Id3v2len = ptr[6]<<7 | ptr[7];
        Id3v2len = Id3v2len<<7 | ptr[8];
        Id3v2len = Id3v2len<<7 | ptr[9];
        Id3v2len += 10;
        if (ptr[5] & 0x10)
            Id3v2len += 10;
    }
    return Id3v2len;
}

cdx_int32 ID3PsrEncodingChange(Id3ParserImplS *id3)
{
    cdx_int32 i;
    cdx_uint8* psrc = 0;
    cdx_uint8* pdrc = 0;
    if(id3->mtitleCharEncode == A_AUDIO_FONTTYPE_ISOIEC8859_1)
    {
        psrc = (cdx_uint8*)(id3->mtitle + id3->mtitle_sz-1);
        pdrc = (cdx_uint8*)(id3->mtitle + 2*(id3->mtitle_sz-1));
        for(i=0;i<id3->mtitle_sz;i++)
        {
            if(*psrc<128)
            {
                *pdrc = *psrc;
                *(pdrc+1) = 0;
            }
            else
            {
                *(pdrc+1) = (windows1251_unicode[*psrc - 128]>>8) &0x00ff;
                *pdrc = windows1251_unicode[*psrc - 128] &0x00ff;
            }
            psrc--;
            pdrc -= 2;
        }
        id3->mtitle_sz *=2;
        id3->mtitleCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
    }
    if(id3->mauthorCharEncode==A_AUDIO_FONTTYPE_ISOIEC8859_1)
    {
        psrc = (cdx_uint8*)(id3->mauthor + id3->mauthor_sz-1);
        pdrc = (cdx_uint8*)(id3->mauthor + 2*(id3->mauthor_sz-1));
        for(i=0;i<id3->mauthor_sz;i++)
        {
            if(*psrc<128)
            {
                *pdrc =*psrc;
                *(pdrc+1) = 0;
            }
            else
            {
                *(pdrc+1) = (windows1251_unicode[*psrc - 128]>>8) &0x00ff;
                *pdrc = windows1251_unicode[*psrc - 128] &0x00ff;
            }
            psrc--;
            pdrc-=2;
        }
        id3->mauthor_sz *=2;
        id3->mauthorCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
    }
    if(id3->mAlbumCharEncode==A_AUDIO_FONTTYPE_ISOIEC8859_1)
    {
        psrc = (cdx_uint8*)(id3->mAlbum + id3->mAlbum_sz-1);
        pdrc = (cdx_uint8*)(id3->mAlbum + 2*(id3->mAlbum_sz-1));
        for(i=0;i<id3->mAlbum_sz;i++)
        {
            if(*psrc<128)
            {
                *pdrc =*psrc;
                *(pdrc+1) = 0;
            }
            else
            {
                *(pdrc+1) = (windows1251_unicode[*psrc - 128]>>8) &0x00ff;
                *pdrc = windows1251_unicode[*psrc - 128] &0x00ff;
            }
            psrc--;
            pdrc-=2;
        }
        id3->mAlbum_sz *=2;
        id3->mAlbumCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
    }
    if(id3->mYearCharEncode==A_AUDIO_FONTTYPE_ISOIEC8859_1)
    {
        psrc = (cdx_uint8*)(id3->mYear + id3->mYear_sz-1);
        pdrc = (cdx_uint8*)(id3->mYear + 2*(id3->mYear_sz-1));
        for(i=0;i<id3->mYear_sz;i++)
        {
            if(*psrc<128)
            {
                *pdrc =*psrc;
                *(pdrc+1) = 0;
            }
            else
            {
                *(pdrc+1) = (windows1251_unicode[*psrc - 128]>>8) &0x00ff;
                *pdrc = windows1251_unicode[*psrc - 128] &0x00ff;
            }
            psrc--;
            pdrc-=2;
        }
        id3->mYear_sz *=2;
        id3->mYearCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
    }
    if(id3->mGenreCharEncode==A_AUDIO_FONTTYPE_ISOIEC8859_1)
    {
        psrc = (unsigned char*)(id3->mGenre + id3->mGenre_sz-1);
        pdrc = (unsigned char*)(id3->mGenre + 2*(id3->mGenre_sz-1));
        for(i=0;i<id3->mGenre_sz;i++)
        {
            if(*psrc<128)
            {
                *pdrc =*psrc;
                *(pdrc+1) = 0;
            }
            else
            {
                *(pdrc+1) = (windows1251_unicode[*psrc - 128]>>8) &0x00ff;
                *pdrc = windows1251_unicode[*psrc - 128] &0x00ff;
            }
            psrc--;
            pdrc-=2;
        }
        id3->mGenre_sz *=2;
        id3->mGenreCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
    }
    return 0;
}

cdx_int32 ID3PsrGetLanguageEncoding(char* ulcomm)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(ISO_639_2_Code); i++)
    {
        if(!strcasecmp(ulcomm, ISO_639_2_Code[i].language))
            return ISO_639_2_Code[i].coding;
    }
    return -1;
}

cdx_uint32    ID3PsrShowBs(cdx_int32 skipLen,cdx_int32 ByteLen,Id3ParserImplS *id3)
{//显示从当前位置偏移skipLen 长度（byte）的byte
    cdx_uint32  RetVal = 0;
    cdx_uint8    data;
    cdx_int32   i,ret = 0;
    cdx_uint8    *ptr;
    //cdx_int32    Flen;
    cdx_uint8    ptrdata[16]= {0};
    cdx_int64   offset = CdxStreamTell(id3->stream);

    if(CdxStreamSeek(id3->stream,skipLen,SEEK_CUR))
    {
        CDX_LOGE("%s,l:%d,data no enough!!",__func__,__LINE__);
        return -1;
    }
    ret = CdxStreamRead(id3->stream,ptrdata,ByteLen);
    if(ret < ByteLen)
    {
        CDX_LOGE("%s,l:%d,data no enough!!",__func__,__LINE__);
        return -1;
    }
    ptr = ptrdata;
    for(i=0;i<ByteLen;i++)
    {
        data = *ptr++;
        RetVal = (RetVal<<8)|(data&0xff);
    }
    CdxStreamSeek(id3->stream,offset,SEEK_SET);
    return    RetVal;
}

cdx_int32 ID3PsrSkipBsInByte(int    ByteLen,Id3ParserImplS *id3)
{
    cdx_uint8  temp[SKIPLEN] = {0};
    cdx_int32 totallen = ByteLen;
    cdx_int32 skiplen = (ByteLen>SKIPLEN)? SKIPLEN:ByteLen;
    cdx_int32 retlen = 0;
    while(totallen > 0)
    {
        skiplen = (totallen>SKIPLEN)? SKIPLEN:totallen;
        retlen = CdxStreamRead(id3->stream,temp,skiplen);
        if(retlen < skiplen)
        {
            CDX_LOGE("Skip EOS!!");
            return 0;
        }
        totallen -= retlen;
    }
    return    1;
}

cdx_uint32    ID3PsrGetBsInByte(cdx_int32    ByteLen,Id3ParserImplS *id3,cdx_int32 move_ptr)
{
    cdx_uint32    RetVal;
    cdx_uint8      data;
    cdx_uint8      tempbuf[64] = {0};
    cdx_int32     i;
    //cdx_int32            readLen;
    cdx_int64     offset = 0;

    offset = CdxStreamTell(id3->stream);
    RetVal = 0;
    CdxStreamRead(id3->stream, tempbuf,ByteLen);
    for (i=0;i<ByteLen;i++)
    {
        data = tempbuf[i];//*BSINFO->bsbufptr++;
        RetVal = (RetVal<<8)|(data&0xff);
    }
    if(!move_ptr)
        CdxStreamSeek(id3->stream,offset,SEEK_SET);
    return RetVal;
}

static void ID3PsrReadNunicode(cdx_int32 *FrameLen, char **Frame, Id3ParserImplS *id3,
                               int frameTLen)
{
    cdx_uint8 data;
    cdx_int32 i;
    *FrameLen = frameTLen -1 ;
    *Frame = (char *)id3->mInforBuf;

    if(id3->mInforBufLeftLength< *FrameLen)
        return;
    id3->mInforBufLeftLength -= *FrameLen;

    for(i=0;i<*FrameLen;i++)
    {
        data= (cdx_uint8)ID3PsrGetBsInByte(1,id3,1);
        *id3->mInforBuf++= data;
    }
    return;
}

static void ID3PsrReadunicode(cdx_int32 *FrameLen, char **Frame, Id3ParserImplS *id3,
                              int frameTLen)
{
    cdx_int32 UnicodeBOM;
    cdx_uint8 data,data1,data2;
    cdx_int32 i;
    if(frameTLen<4)
    {
        *FrameLen = 0;
        ID3PsrSkipBsInByte(frameTLen-1,id3);
        return;
    }
    *FrameLen = frameTLen -1-2 ;//BOM LEN
    *Frame = (char *)id3->mInforBuf;

    if(id3->mInforBufLeftLength< *FrameLen)
        return;
    id3->mInforBufLeftLength -= *FrameLen;
    UnicodeBOM = ID3PsrGetBsInByte(2,id3,1);

    for(i=0;i<(*FrameLen)/2;i++)
    {
        data= (cdx_uint8)ID3PsrGetBsInByte(1,id3,1);
        data1= (cdx_uint8)ID3PsrGetBsInByte(1,id3,1);
        if(UnicodeBOM == 0xfeff)
        {
            data2=data;
            data=data1;
            data1=data2;
        }
        *id3->mInforBuf++= data;
        *id3->mInforBuf++= data1;
    }
    ID3PsrSkipBsInByte(*FrameLen&0x01,id3);
    return;
}

cdx_int32    ID3PsrGeTID3_V2(Id3ParserImplS *parser)  //IF HAVE RETURN ID3 tag length
{
    Id3ParserImplS *id3;
    cdx_int32 Id3v2len;
    cdx_int32 Id3Version=0, myId3Version = 0;;
    cdx_uint32    Id3v2lenRet=0;
    cdx_uint32 BsVal;
    cdx_uint32 temp[10];

    cdx_uint8   data;
    cdx_int32  i;
    cdx_int32  frameinfoseg=0;
    cdx_int32  TxtEncodeflag;
    cdx_int32  ulLanguage_encoding = 0;
    struct Id3Pic* thiz = NULL,*tmp = NULL;

    id3 = (Id3ParserImplS *)parser;
    if(id3->forceStop == 1)
        goto ForceExit;
    id3->mInforBufLeftLength = INFLEN;//less than 8k
    id3->mInforBuf = id3->mInfor;
    memset(id3->mInforBuf,0,id3->mInforBufLeftLength);

    struct {
        cdx_uint32 UID;
        cdx_int32  length;
        cdx_int16  flag;

    }ID3FRAME;

    BsVal = ID3PsrGetBsInByte(4,id3,0);
    BsVal >>=8;
    if(BsVal == 0x494433/*ID3*/)  //ID3v2/file identifier   "ID3"
    {
        BsVal = ID3PsrGetBsInByte(3,id3,1);

        Id3Version=ID3PsrGetBsInByte(2,id3,1);//ID3v2 version
        Id3Version &= 0xff00;//ver and revision,now use ver
        if(Id3Version==0x200)
            Id3Version =1;
        else if((Id3Version ==0x300) ||(Id3Version ==0x400))
        {
            if(Id3Version ==0x400)
                myId3Version = 4;
            Id3Version = 0;
        }
        else//for error and future;20100825lszhang
            Id3Version = 0;

        temp[5] = ID3PsrGetBsInByte(1,id3,1);//ID3v2 flags
        temp[3] = ID3PsrGetBsInByte(1,id3,1);
        temp[2] = ID3PsrGetBsInByte(1,id3,1);
        temp[1] = ID3PsrGetBsInByte(1,id3,1);
        temp[0] = ID3PsrGetBsInByte(1,id3,1);
        Id3v2len = temp[3]<<7 | temp[2];
        Id3v2len = Id3v2len<<7 | temp[1];
        Id3v2len = Id3v2len<<7 | temp[0];
        Id3v2lenRet = Id3v2len;
        if (temp[5] & 0x10)
           Id3v2lenRet += 10;
        while((Id3v2len>=10)&&(frameinfoseg<ID3TAGNUM))
        {
            if(id3->forceStop == 1)
            {
                goto ForceExit;
            }
            if(Id3Version == 1) //ID3V2.2
            {
                ID3FRAME.UID = ID3PsrGetBsInByte(3,id3,1);
                ID3FRAME.length = ID3PsrGetBsInByte(3,id3,1);
                //ID3FRAME.flag = ID3PsrGetBsInByte(1,AIF);
                Id3v2len -=6;
            }
            else
            {
                ID3FRAME.UID = ID3PsrGetBsInByte(4,id3,1);
                ID3FRAME.length = ID3PsrGetBsInByte(4,id3,1);
                if(myId3Version == 4)
                {
                    if(ID3Switch2SyncsafeInteger(&ID3FRAME.length) == CDX_FALSE)
                    {
                        CDX_LOGE("ID3v4 detect frame lenth has occured unknown errors!");
                        return -1;
                    }
                }
                ID3FRAME.flag = ID3PsrGetBsInByte(2,id3,1);
                Id3v2len -=10;
            }
            if(ID3FRAME.length<0)break;
            if(ID3FRAME.length==0)continue;

            if(ID3FRAME.UID == ID3UniqueTag[Id3Version][0]/* 0x54495432*/) /*TIT2*/
            {    //read tile
                frameinfoseg++;
                TxtEncodeflag=ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                switch(TxtEncodeflag)
                {
                    case 0:
                        ID3PsrReadNunicode(&id3->mtitle_sz,&id3->mtitle,id3,ID3FRAME.length);
                        if(id3->mInforBufLeftLength< (ID3FRAME.length-1))
                            break;
                        id3->mInforBufLeftLength -= (ID3FRAME.length-1);
                        id3->mInforBuf          += (ID3FRAME.length-1);
                        break;
                    case 1:
                        id3->mtitleCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
                        ID3PsrReadunicode(&id3->mtitle_sz,&id3->mtitle,id3,ID3FRAME.length);
                        break;
                    case 2:
                        id3->mtitleCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;//2;
                        ID3PsrReadunicode(&id3->mtitle_sz,&id3->mtitle,id3,ID3FRAME.length);
                        break;
                    case 3:
                        id3->mtitleCharEncode = A_AUDIO_FONTTYPE_UTF_8;
                        ID3PsrReadNunicode(&id3->mtitle_sz,&id3->mtitle,id3,ID3FRAME.length);
                        break;
                    default:
                        ID3PsrSkipBsInByte(ID3FRAME.length-1,id3);
                }
                Id3v2len -=ID3FRAME.length;
            }
            else if(ID3FRAME.UID == ID3UniqueTag[Id3Version][1]/*0x54504531*/) /*TPE1*/
            {
                frameinfoseg++;
                TxtEncodeflag=ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                switch(TxtEncodeflag)
                {
                    case 0:
                        ID3PsrReadNunicode(&id3->mauthor_sz,&id3->mauthor,id3,ID3FRAME.length);
                        if(id3->mInforBufLeftLength< (ID3FRAME.length-1))
                            break;
                        id3->mInforBufLeftLength -= (ID3FRAME.length-1);
                        id3->mInforBuf          += (ID3FRAME.length-1);
                        break;
                    case 1:
                        id3->mauthorCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
                        ID3PsrReadunicode(&id3->mauthor_sz,&id3->mauthor,id3,ID3FRAME.length);
                        break;
                    case 2:
                        id3->mauthorCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;//2;
                        ID3PsrReadunicode(&id3->mauthor_sz,&id3->mauthor,id3,ID3FRAME.length);
                        break;
                    case 3:
                        id3->mauthorCharEncode = A_AUDIO_FONTTYPE_UTF_8;
                        ID3PsrReadNunicode(&id3->mauthor_sz,&id3->mauthor,id3,ID3FRAME.length);
                        break;
                    default:
                        ID3PsrSkipBsInByte(ID3FRAME.length-1,id3);
                }
                Id3v2len -= ID3FRAME.length;
            }
            else if(ID3FRAME.UID == ID3UniqueTag[Id3Version][2]/*0x54414C42*/) /*TALB*/
            {
                frameinfoseg++;
                TxtEncodeflag=ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                switch(TxtEncodeflag)
                {
                    case 0:
                        ID3PsrReadNunicode(&id3->mAlbum_sz,&id3->mAlbum,id3,ID3FRAME.length);
                        if(id3->mInforBufLeftLength< (ID3FRAME.length-1))
                            break;
                        id3->mInforBufLeftLength -= (ID3FRAME.length-1);
                        id3->mInforBuf          += (ID3FRAME.length-1);
                        break;
                    case 1:
                        id3->mAlbumCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
                        ID3PsrReadunicode(&id3->mAlbum_sz,&id3->mAlbum,id3,ID3FRAME.length);
                        break;
                    case 2:
                        id3->mAlbumCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;//2;
                        ID3PsrReadunicode(&id3->mAlbum_sz,&id3->mAlbum,id3,ID3FRAME.length);
                        break;
                    case 3:
                        id3->mAlbumCharEncode = A_AUDIO_FONTTYPE_UTF_8;
                        ID3PsrReadNunicode(&id3->mAlbum_sz,&id3->mAlbum,id3,ID3FRAME.length);
                        break;
                    default:
                        ID3PsrSkipBsInByte(ID3FRAME.length-1,id3);
                }
                Id3v2len -=ID3FRAME.length;
            }
            else if(ID3FRAME.UID == ID3UniqueTag[Id3Version][3]/*0x54594552*/) /*TYER*/
            {
                frameinfoseg++;
                TxtEncodeflag=ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                switch(TxtEncodeflag)
                {
                    case 0:
                        ID3PsrReadNunicode(&id3->mYear_sz,&id3->mYear,id3,ID3FRAME.length);
                        if(id3->mInforBufLeftLength< (ID3FRAME.length-1))
                            break;
                        id3->mInforBufLeftLength -= (ID3FRAME.length-1);
                        id3->mInforBuf          += (ID3FRAME.length-1);
                        break;
                    case 1:
                        id3->mYearCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
                        ID3PsrReadunicode(&id3->mYear_sz,&id3->mYear,id3,ID3FRAME.length);
                        break;
                    case 2:
                        id3->mYearCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;//2;
                        ID3PsrReadunicode(&id3->mYear_sz,&id3->mYear,id3,ID3FRAME.length);
                        break;
                    case 3:
                        id3->mYearCharEncode = A_AUDIO_FONTTYPE_UTF_8;
                        ID3PsrReadNunicode(&id3->mYear_sz,&id3->mYear,id3,ID3FRAME.length);
                        break;
                    default:
                        ID3PsrSkipBsInByte(ID3FRAME.length-1,id3);
                }
                Id3v2len -=ID3FRAME.length;
            }
            else if(ID3FRAME.UID == ID3UniqueTag[Id3Version][4]/*0x54434f4e*/) /*TCON*/
            {
                cdx_uint8 *GenreTemp = id3->mInforBuf;//for use AIF->mInforBuf buffer
                cdx_int32 GenreLen,ReadLen=10;
                cdx_uint32 GenreIndex;

                frameinfoseg++;
                TxtEncodeflag=ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                GenreLen = ID3FRAME.length-1;
                Id3v2len -=ID3FRAME.length;
                ReadLen = GenreLen;
                if(ReadLen>id3->mInforBufLeftLength)
                {
                    ReadLen = id3->mInforBufLeftLength;
                }
                if(ReadLen == 0)
                {
                    continue;
                }
                for(i=0;i<ReadLen;i++)
                {
                    data= (char )ID3PsrGetBsInByte(1,id3,1);
                    GenreTemp[i]= data;
                }
                GenreLen -= ReadLen;
                ID3PsrSkipBsInByte(GenreLen,id3);

                if(GenreTemp[0]=='(')
                {
                    switch(TxtEncodeflag)
                    {
                        case 0:
                            break;
                        case 1:
                        case 3:
                            for(i=0;i<((ReadLen-2)/2);i++)
                            {
                                GenreTemp[i]= GenreTemp[2*i+2];//skip BOM
                            }
                            ReadLen = (ReadLen-2)/2;
                            break;
                        case 2:
                            for(i=0;i<((ReadLen-2)/2);i++)
                            {
                                GenreTemp[i]= GenreTemp[2*i+1+2];//skip BOM
                            }
                            ReadLen = (ReadLen-2)/2;
                            break;
                    }
                    if((GenreTemp[1]=='R')&&(GenreTemp[2]=='X'))
                        GenreIndex = 148;
                    else if((GenreTemp[1]=='C')&&(GenreTemp[2]=='R'))
                        GenreIndex = 149;
                    else
                    {
                        i = 1;
                        GenreIndex = 0;
                        while((i<ReadLen)&&(GenreIndex<148+2))
                        {
                            if(GenreTemp[i]==')')
                                break;
                            GenreIndex *=10;
                            GenreIndex += (int)((GenreTemp[i]-0x30)&0xff);
                            i++;
                        }
                        if(GenreIndex<148+2)
                        {
                            id3->mGenreCharEncode = A_AUDIO_FONTTYPE_ISOIEC8859_1;
                            id3->mGenre_sz = 22;
                            id3->mGenre = (char *)id3->mInforBuf;
                            if(id3->mInforBufLeftLength<22)
                                return 0;
                            id3->mInforBufLeftLength -=22;
                            memcpy(id3->mGenre,ID3V1GENRE[GenreIndex],22);
                            id3->mGenre_sz = strlen(id3->mGenre);
                            id3->mInforBuf += 22;
                        }
                    }
                }
                else
                {
                    id3->mGenreCharEncode = (cdx_audio_fonttype_e)TxtEncodeflag;
                    id3->mGenre_sz =  ReadLen;
                    id3->mGenre = (char *)id3->mInforBuf;
                    i = 0;
                    if((TxtEncodeflag==1)||(TxtEncodeflag==2))
                    {
                        i=2;
                        id3->mGenre_sz -=2;
                    }
                    id3->mInforBufLeftLength -= ReadLen - i;
                    for(;i<ReadLen;i++)
                    {
                        *id3->mInforBuf++ = GenreTemp[i];
                    }
                }
            }
            else if(ID3FRAME.UID == ID3UniqueTag[Id3Version][5]/*0x41504943*/) /*APIC*/
            {
                char *p = 0;
                char *GenreTemp = (char *)id3->mInforBuf;//for use AIF->mInforBuf buffer
                cdx_id3_image_info_t image;
                frameinfoseg++;
                TxtEncodeflag = ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                Id3v2len -=ID3FRAME.length;
                ID3FRAME.length -= 1;
                memset(&image,0,sizeof(cdx_id3_image_info_t));

                if(ID3FRAME.length == 0)
                    continue;

                i = 0;
                do
                {
                    if(id3->forceStop == 1)
                    {
                        goto ForceExit;
                    }
                    data= (char )ID3PsrGetBsInByte(1,id3,1);
                    GenreTemp[i]= data;
                    i++;
                }while(data != 0);
                p = GenreTemp;
                if(!strcmp(p, IMAGE_MIME_TYPE_BMP))
                {
                   image.img_format = IMG_FORMAT_BMP;
                }
                else if(!strcmp(p, IMAGE_MIME_TYPE_JPEG))
                {
                   image.img_format = IMG_FORMAT_JPG;
                }
                else if(!strcmp(p, IMAGE_MIME_TYPE_PNG))
                {
                   image.img_format = IMG_FORMAT_PNG;
                }
                else
                {
                   image.img_format = IMG_FORMAT_UNSUPPORT;
                }
                ID3FRAME.length -=  i;
                if(ID3FRAME.length == 0)
                    continue;
                image.pic_type = ID3PsrGetBsInByte(1,id3,1);
                ID3FRAME.length -= 1;
                if(ID3FRAME.length == 0)
                    continue;
                i = 0;
                do
                {
                    if(id3->forceStop == 1)
                    {
                        goto ForceExit;
                    }
                    data= (cdx_uint8)ID3PsrGetBsInByte(1,id3,1);
                    if((TxtEncodeflag ==1)||(TxtEncodeflag ==2))
                    {
                        data |= (cdx_uint8)ID3PsrGetBsInByte(1,id3,1);
                        i++;
                    }
                    i++;
                }while(data != 0);

                switch(TxtEncodeflag)
                {
                    case 0:
                        break;
                    case 1:
                        id3->mAPicCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;
                        break;
                    case 2:
                        id3->mAPicCharEncode = A_AUDIO_FONTTYPE_UTF_16LE;//2;
                        break;
                    case 3:
                        id3->mAPicCharEncode = A_AUDIO_FONTTYPE_UTF_8;
                        break;
                    default:
                        ;
                }
                ID3FRAME.length -= i;
                if(ID3FRAME.length == 0) continue;
                id3->mAPic_sz = sizeof(cdx_id3_image_info_t);
                CDX_LOGE("Notice!U should load albumart 'APIC' here   lenth:%d",ID3FRAME.length);
                image.length = ID3FRAME.length;
#if 1
                if(id3->pAlbumArtid == 0)
                {
                    struct Id3Pic* thizPic = malloc(sizeof(struct Id3Pic));
                    memset(thizPic,0x00,sizeof(struct Id3Pic));
                    thizPic->lenth = image.length;
                    thizPic->addr = (cdx_uint8*)malloc(image.length);

                    image.FileLocation = CdxStreamTell(id3->stream);
                    CdxStreamRead(id3->stream,thizPic->addr,image.length);
                    CdxStreamSeek(id3->stream,image.FileLocation,SEEK_SET);

                    thizPic->father = id3->pAlbumArt;
                    id3->pAlbumArt = thizPic;
                    id3->pAlbumArtid++;
                }
#endif
                if(image.FileLocation < 0)image.FileLocation = 0;
                id3->mInforBufLeftLength -= 4-((intptr_t)(id3->mInforBuf)&0x3);
                id3->mInforBuf += 4-((intptr_t)(id3->mInforBuf)&0x3);
                id3->mAPic = (cdx_id3_image_info_t*)(id3->mInforBuf);
                if(id3->mInforBufLeftLength < id3->mAPic_sz)
                    break;
                id3->mInforBufLeftLength -= id3->mAPic_sz;
                memcpy(id3->mAPic,&image,id3->mAPic_sz);
                id3->mInforBuf += id3->mAPic_sz;
                ID3PsrSkipBsInByte(ID3FRAME.length,id3);
            }
            else if((ID3FRAME.UID == ID3UniqueTag[Id3Version][6]/*0x434f4d4d*/) /*COMM*/
                    ||(ID3FRAME.UID == ID3UniqueTag[Id3Version][6]/*0x544c414e*/) /*TLAN*/)
            {
                cdx_uint8 ulcomm[4] ={0,0,0,0} ;
                frameinfoseg++;
                TxtEncodeflag=ID3PsrGetBsInByte(1,id3,1);//TEXT ENCODERing 00
                ulcomm[0] = ID3PsrGetBsInByte(1,id3,1);
                ulcomm[1] = ID3PsrGetBsInByte(1,id3,1);
                ulcomm[2] = ID3PsrGetBsInByte(1,id3,1);
                ulLanguage_encoding = ID3PsrGetLanguageEncoding((char *)ulcomm);
                ID3PsrSkipBsInByte(ID3FRAME.length-1-3,id3);
                Id3v2len -=ID3FRAME.length;
            }
            else
            {
                if(Id3v2len<ID3FRAME.length)
                    ID3FRAME.length = Id3v2len;
                ID3PsrSkipBsInByte(ID3FRAME.length,id3);
                Id3v2len -=ID3FRAME.length;
            }
        }
        ID3PsrSkipBsInByte(Id3v2len,id3);
        Id3v2lenRet +=10;
    }
    if(ulLanguage_encoding==A_AUDIO_FONTTYPE_WINDOWS_1251)
    {
            ID3PsrEncodingChange(id3);
    }
    //for other ID3
    BsVal = ID3PsrGetBsInByte(4,id3,0);
    BsVal >>=8;
    while(BsVal == 0x494433/*ID3*/)
    {
        cdx_uint8 *p = (cdx_uint8*)temp;
        if(id3->forceStop == 1)
        {
            goto ForceExit;
        }
        for(i=0;i<10;i++)
        {
            *p++ = ID3PsrShowBs(i,1,id3);
        }
        i = ID3PsrGETID3Len((cdx_uint8*)temp,10);
        if(i<0)
        {
            break;
        }
        ID3PsrSkipBsInByte(i,id3);
        Id3v2lenRet +=i;
        BsVal = ID3PsrGetBsInByte(4,id3,0);
        BsVal >>=8;
    }
    return    Id3v2lenRet;
ForceExit:
    thiz = NULL;
    tmp = NULL;
    thiz = id3->pAlbumArt;
    id3->pAlbumArt = NULL;
    while(thiz != NULL)
    {
        if(thiz->addr!=NULL)
        {
            free(thiz->addr);
            CDX_LOGE("FREE PIC");
            thiz->addr = NULL;
        }
        tmp = thiz;
        thiz = thiz->father;
        if(tmp!=NULL)
        {
            free(tmp);
            id3->pAlbumArtid--;
            CDX_LOGE("FREE PIC COMPLETE pAlbumArtid:%d",id3->pAlbumArtid);
            tmp = NULL;
        }
    }
    return -1;
}

static int Id3Init(CdxParserT *id3_impl)
{
    cdx_int32 ret = 0;
    cdx_int32 offset=0;
    cdx_int32 tmpFd = 0;
    unsigned char headbuf[10] = {0};
    struct Id3ParserImplS *impl = NULL;

    impl = (Id3ParserImplS*)id3_impl;
    #if 1
    offset = ID3PsrGeTID3_V2(impl);
    if(offset < 0)
    {
        CDX_LOGE("ID3PsrGeTID3_V2 Unknown errors!");
        return -1;
    }
    #else
    CdxStreamRead(impl->stream,headbuf,10);
    impl->file_offset = 10;
    if(!memcmp(headbuf,ID3TAG,3))//for id3
    {
        CDX_LOGE("ID3 SKIPING!");
        impl->Id3v2len = ((cdx_int32)(headbuf[6]&0x7f))<<7 | ((cdx_int32)(headbuf[7]&0x7f));
        impl->Id3v2len = impl->Id3v2len<<7 | ((cdx_int32)(headbuf[8]&0x7f));
        impl->Id3v2len = impl->Id3v2len<<7 | ((cdx_int32)(headbuf[9]&0x7f));
        offset = impl->Id3v2len;
        impl->Id3v2len +=10;
    }
    #endif
    CDX_LOGD("SKIP TO POST ID3 LOCATION:%d!",offset);
    /*
    ret = CdxStreamSeek(impl->stream, offset, SEEK_CUR);
    if(ret==-1){
        CDX_LOGE("Skip id3 byte error!");
        goto OPENFAILURE;
    }*/
    impl->file_offset += offset;

    //reopen parser
    memset(&impl->cdxDataSource, 0x00, sizeof(impl->cdxDataSource));
    CdxStreamGetMetaData(impl->stream,METEDATAKEY,(void **)&impl->keyinfo);
    if(strncmp(impl->keyinfo, "file://", 7) == 0 || strncmp(impl->keyinfo, "fd://", 5) == 0)
    {
        ret = sscanf(impl->keyinfo, "fd://%d?offset=%lld&length=%lld", &tmpFd, &impl->fdoffset,
                     &impl->file_size);
        ret = sprintf(impl->newurl, "fd://%d?offset=%lld&length=%lld", tmpFd,
                      impl->fdoffset+impl->file_offset, impl->file_size - impl->file_offset);
    }
    else if(strncmp(impl->keyinfo, "http://", 7) == 0 || strncmp(impl->keyinfo, "https://", 8)== 0)
    {
        strcpy(impl->newurl, impl->keyinfo);
        impl->cdxDataSource.offset = impl->file_offset;
    }

    logd("impl->newurl(%s), impl->file_offset(%lld)", impl->newurl, impl->file_offset);
    impl->cdxDataSource.uri = impl->newurl;

    CdxStreamProbeDataT *probeData = CdxStreamGetProbeData(impl->stream);
    if (probeData->len > impl->file_offset)
    {
        probeData->len -= impl->file_offset;
        memmove(probeData->buf, probeData->buf + impl->file_offset, probeData->len);
        ret = CdxParserOpen(impl->stream, NO_NEED_DURATION, &impl->lock,
            &impl->forceStop, &impl->child, NULL);
        if (ret == 0)
        {
            impl->shareStreamWithChild = 1;
            impl->mErrno = PSR_OK;
            return 0;
        }
        else if (impl->child)
        {
            CdxParserClose(impl->child);
        }
    }

    ret = CdxParserPrepare(&impl->cdxDataSource, NO_NEED_DURATION, &impl->lock, &impl->forceStop,
        &impl->child, &impl->childStream, NULL, NULL);
    if(ret < 0)
    {
        CDX_LOGE("CdxParserPrepare fail");
        goto OPENFAILURE;
    }

    impl->mErrno = PSR_OK;
    return 0;
OPENFAILURE:
    CDX_LOGE("Id3OpenThread fail!!!");
    impl->mErrno = PSR_OPEN_FAIL;
    return -1;
}

static cdx_int32 __Id3ParserControl(CdxParserT *parser, cdx_int32 cmd, void *param)
{
    struct Id3ParserImplS *impl = NULL;
    impl = (Id3ParserImplS*)parser;
    (void)param;
    if(!impl->child)
        return CDX_SUCCESS;
    switch (cmd)
    {
        case CDX_PSR_CMD_DISABLE_AUDIO:
        case CDX_PSR_CMD_DISABLE_VIDEO:
        case CDX_PSR_CMD_SWITCH_AUDIO:
            break;
        case CDX_PSR_CMD_SET_FORCESTOP:
            impl->forceStop = 1;
            if(impl->child)
            {
                CdxParserForceStop(impl->child);
            }
            else if(impl->childStream)
            {
                CdxStreamForceStop(impl->childStream);
            }
            break;
        case CDX_PSR_CMD_CLR_FORCESTOP:
            impl->forceStop = 0;
            if(impl->child)
            {
                CdxParserClrForceStop(impl->child);
            }
            else if(impl->childStream)
            {
                CdxStreamClrForceStop(impl->childStream);
            }
            break;
        default :
            CDX_LOGD("Default success to child processing...");
            CdxParserControl(impl->child, cmd, param);
            break;
    }
    impl->flags = cmd;
    return CDX_SUCCESS;
}

static cdx_int32 __Id3ParserPrefetch(CdxParserT *parser, CdxPacketT *pkt)
{
    cdx_int32 ret = CDX_FAILURE;
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);
    if(!impl->child)
        return ret;
    ret = CdxParserPrefetch(impl->child,pkt);
    impl->mErrno = CdxParserGetStatus(impl->child);
    return ret;
}

static cdx_int32 __Id3ParserRead(CdxParserT *parser, CdxPacketT *pkt)
{
    cdx_int32 ret = CDX_FAILURE;
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);
    if(!impl->child)
        return ret;
    ret = CdxParserRead(impl->child,pkt);
    impl->mErrno = CdxParserGetStatus(impl->child);
    return ret;
}

static cdx_int32 __Id3ParserGetMediaInfo(CdxParserT *parser, CdxMediaInfoT *mediaInfo)
{
    cdx_int32 ret = CDX_FAILURE;
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);

    /*load the id3 infomation*/
    if(impl->mAlbum_sz < 64 && impl->mAlbum)
    {
        memcpy(mediaInfo->album,impl->mAlbum,impl->mAlbum_sz);
        mediaInfo->albumsz = impl->mAlbum_sz;
        mediaInfo->albumCharEncode = (cdx_int32)impl->mAlbumCharEncode;
    }
    if(impl->mauthor_sz < 64 && impl->mauthor)
    {
        memcpy(mediaInfo->author,impl->mauthor,impl->mauthor_sz);
        mediaInfo->authorsz = impl->mauthor_sz;
        mediaInfo->authorCharEncode = (cdx_int32)impl->mauthorCharEncode;
    }
    if(impl->mYear_sz < 64 && impl->mYear)
    {
        memcpy(mediaInfo->year,impl->mYear,impl->mYear_sz);
        mediaInfo->yearsz = impl->mYear_sz;
        mediaInfo->yearCharEncode = impl->mYearCharEncode;
    }
    if(impl->mGenre_sz < 64 && impl->mGenre)
    {
        memcpy(mediaInfo->genre,impl->mGenre,impl->mGenre_sz);
        mediaInfo->genresz = impl->mGenre_sz;
        mediaInfo->genreCharEncode = impl->mGenreCharEncode;
    }
    if(impl->mtitle_sz < 64 && impl->mtitle)
    {
        memcpy(mediaInfo->title,impl->mtitle,impl->mtitle_sz);
        mediaInfo->titlesz = impl->mtitle_sz;
        mediaInfo->titleCharEncode = impl->mtitleCharEncode;
    }
    /*I have APIC length and location, do u wanna it????*/
    if(impl->pAlbumArt != NULL && impl->pAlbumArt->lenth>0)
    {
        CDX_LOGE("How many pic ?: %d",impl->pAlbumArtid);
        mediaInfo->nAlbumArtBufSize = impl->pAlbumArt->lenth;
        mediaInfo->pAlbumArtBuf = impl->pAlbumArt->addr;
    }
    /*load the music infomation*/
    if(!impl->child)
        return ret;
    ret = CdxParserGetMediaInfo(impl->child,mediaInfo);
    impl->mErrno = CdxParserGetStatus(impl->child);
    return ret;
}

static cdx_int32 __Id3ParserSeekTo(CdxParserT *parser, cdx_int64 timeUs)
{
    cdx_int32 ret = CDX_FAILURE;
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);
    if(!impl->child)
        return ret;
    ret = CdxParserSeekTo(impl->child,timeUs);
    impl->mErrno = CdxParserGetStatus(impl->child);
    return ret;
}

static cdx_uint32 __Id3ParserAttribute(CdxParserT *parser)
{
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);
    return CdxParserAttribute(impl->child);
}
#if 0
static cdx_int32 __Id3ParserForceStop(CdxParserT *parser)
{
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);
    return CdxParserForceStop(impl->child);
}
#endif
static cdx_int32 __Id3ParserGetStatus(CdxParserT *parser)
{
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);

    return impl->child?CdxParserGetStatus(impl->child):impl->mErrno;
}

static cdx_int32 __Id3ParserClose(CdxParserT *parser)
{
    struct Id3ParserImplS *impl = NULL;
    impl = CdxContainerOf(parser, struct Id3ParserImplS, base);
#if 1
    struct Id3Pic* thiz = NULL,*tmp = NULL;
    thiz = impl->pAlbumArt;
    impl->pAlbumArt = NULL;
    while(thiz != NULL)
    {
        if(thiz->addr!=NULL)
        {
            free(thiz->addr);
            CDX_LOGE("FREE PIC");
            thiz->addr = NULL;
        }
        tmp = thiz;
        thiz = thiz->father;
        if(tmp!=NULL)
        {
            free(tmp);
            impl->pAlbumArtid--;
            CDX_LOGE("FREE PIC COMPLETE impl->pAlbumArtid:%d",impl->pAlbumArtid);
            tmp = NULL;
        }
    }
#endif
    if(impl->child)
    {
        CdxParserClose(impl->child);
    }
    else if(impl->childStream)
    {
        CdxStreamClose(impl->childStream);
    }
    pthread_mutex_destroy(&impl->lock);
    if (impl->shareStreamWithChild == 0)
        CdxStreamClose(impl->stream);
    CdxFree(impl);
    return CDX_SUCCESS;
}

static struct CdxParserOpsS id3ParserOps =
{
    .control = __Id3ParserControl,
    .prefetch = __Id3ParserPrefetch,
    .read = __Id3ParserRead,
    .getMediaInfo = __Id3ParserGetMediaInfo,
    .seekTo = __Id3ParserSeekTo,
    .attribute = __Id3ParserAttribute,
    .getStatus = __Id3ParserGetStatus,
    .close = __Id3ParserClose,
    .init = Id3Init
};

static cdx_uint32 __Id3ParserProbe(CdxStreamProbeDataT *probeData)
{
    CDX_CHECK(probeData);
    if(probeData->len < 10)
    {
        CDX_LOGE("Probe ID3_header data is not enough.");
        return 0;
    }

    if(memcmp(probeData->buf,ID3TAG,3))
    {
        CDX_LOGE("id3 probe failed.");
        return 0;
    }
    return 100;
}

static CdxParserT *__Id3ParserOpen(CdxStreamT *stream, cdx_uint32 flags)
{
    cdx_int32 ret = 0;
    struct Id3ParserImplS *impl;
    impl = CdxMalloc(sizeof(*impl));

    memset(impl, 0x00, sizeof(*impl));
    ret = pthread_mutex_init(&impl->lock, NULL);
    CDX_FORCE_CHECK(ret == 0);
    impl->stream = stream;
    impl->base.ops = &id3ParserOps;
    (void)flags;
    //ret = pthread_create(&impl->openTid, NULL, Id3OpenThread, (void*)impl);
    //CDX_FORCE_CHECK(!ret);
    impl->mErrno = PSR_INVALID;

    return &impl->base;
}

struct CdxParserCreatorS id3ParserCtor =
{
    .probe = __Id3ParserProbe,
    .create = __Id3ParserOpen
};
