// ****************************************************************************
//  characters.cc                                                DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Character tables loaded from a characters file
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "characters.h"

#include "unit.h"
#include "user_interface.h"
#include "utf8.h"



#define CFILE   "config/characters.csv"


// ============================================================================
//
//   Read data from the characters file
//
// ============================================================================

symbol_g characters_file::next()
// ----------------------------------------------------------------------------
//   Find the next file entry if there is one
// ----------------------------------------------------------------------------
{
    bool     quoted = false;
    symbol_g result = nullptr;
    scribble scr;

    while (valid())
    {
        char c = getchar();
        if (!c)
            break;

        if (c == '"')
        {
            if (quoted && peek() == '"') // Treat double "" as a data quote
            {
                c = getchar();
                byte *buf = rt.allocate(1);
                *buf = byte(c);
            }
            else
            {
                quoted = !quoted;
            }
            if (!quoted)
            {
                result = symbol::make(scr.scratch(), scr.growth());
                return result;
            }
        }
        else if (quoted)
        {
            byte *buf = rt.allocate(1);
            *buf = byte(c);
        }
    }

    return result;
}



// ============================================================================
//
//   Character lookup
//
// ============================================================================

static const cstring basic_characters[] =
// ----------------------------------------------------------------------------
//   List of basic characters
// ----------------------------------------------------------------------------
//   clang-format off
{
    //           123456123456123456
    "",         ("AÀÁÂÃaàáâãÄÅĀĂĄäåāăąǍÆǼǺ@ǎæǽǻªΑΆАЯẠαάаяạ"
                 "ẢẤẦẨẪảấầẩẫẬẮẰẲẴậắằẳẵẶặ"),
    "",         "BΒБВЪЬbβбвъьßẞЫы",
    "",         "CÇĆĈĊcçćĉċČĆĈĊČčćĉċčСΓсγ©¢ℂ℅Ⓒℂ",
    "",         "DÐĎĐΔДdðďđδдЂђ₫∂",
    "",         ("EÈÉÊËeèéêëĒĔĖĘĚěēĕėęЀЁЄЄЭѐёєєэЕΕΈΗΉεέϵηήеⒺ"
                 "ẸẺẼẾỀẹẻẽếềỂỄỆÆ€ểễệæ&"),
    "",         "FΦФfφфϕƒ₣",
    "",         "GĜĞĠĢgĝğġģΓГЃҐγгѓґℊ",
    "",         "HĤĦΗΉХhĥħηήхЧШЩчшщℎℏ",
    "",         "IÌÍÎÏiìíîïĨĪĬĮİĩīĭįıǏĲΙΊΪǐĳιίΐΪЇІИЍϊїіиѝЙỈỊйỉị",
    "",         "JĴĲЈjĵĳȷј",
    "",         "KĶΚΧЌКķkκχќкĸ",
    "",         "LĹĻĽĿŁlĺļľŀłΛЛЉλљ₤ℓⓁ",
    "",         "MΜМmµмμ",
    "",         "NÑŃŅŇŊnñńņňŋΝЊНνΰњнŉⁿ№",
    "",         ("OÒÓÔÕoòóôõÖŌŎŐƠöōŏőơǑØǾŒΌǒøǿœόΩΏОỌỎωώоọỏ"
                 "ỐỒỔỖỘốồổỗộỚỜỞỠỢớờởỡợ°0º℅"),
    "",         "PΠПРΨpπпрψϖ¶₧",
    "",         "Qqℚ",
    "",         "RŔŖŘΡРРřrŕŗρрʳℝ",
    "",         "SŚŜŞŠȘsśŝşšșΣЅСσѕс$§ßẞſ",
    "",         "TŢŤŦȚtţťŧțΘΤТÞЋθτтþћЦц℡™",
    "",         ("UÙÚÛÜuùúûüŨŪŬŮŰũūŭůűŲƯǓǕǙųưǔǖǘǛΫΎЎУǜϋύўу"
                 "ỤỦỨỪỬụủứừửỮỰЮữựю"),
    "",         "VВvв",
    "",         "WŴẀẂẄΩwŵẁẃẅω",
    "",         "XΞΧХxξχх",
    "",         "YÝŶŸΥyýÿŷυΎỲỴỶỸύỳỵỷỹΫЫЮЯ¥ϋыюя",
    "",         "ZŹŻŽΖЏzźżžζџЖЗжз",
    "",         "0₀⁰°º",
    "",         "1₁¹¼½",
    "",         "2₂²½",
    "",         "3₃³¾",
    "",         "4₄⁴¼¾",
    "",         "5₅⁵",
    "",         "6₆⁶",
    "",         "7₇⁷",
    "",         "8₈⁸",
    "",         "9₉⁹",
    "",         "‽?¿¡ˀ,.·;!‼",
    "",         "^⁳ˆˇˉ˘˙˚˛˜˝̣ʹ͵",
    "",         "-‐–—―−_‗‾",
    "",         "'\"′″`´‘’‚‛“”„",
    "",         "|†‡",
    "",         "*×·•⨯⋅",
    "",         "/÷⁄",
    "",         ".…",
    "",         "%‰½¼¾℅",
    "",         "<‹«>»›",
    "",         "$€¢£¤¥₣₤₧₫₭₹₺₽ƒ",

    "RPL",      "→⇄Σ∏∆" "≤≠≥∂∫" "ⒸⒺⓁ|?" "ⅈ∡·×÷" "_⁳°′″" "«»{}↑" "ⒹⓧⓇⓈ",
    "Arith",    "+-*/×÷" "<=>≤≠≥" "·%^↑\\±",
    "Math",     ("Σ∏∆∂∫" "πℼ′″°" "ⅈⅉℂℚℝ"
                 "+-±^↑" "*×·∙∡" "/÷%‰⁳"
                 "₀₁₂₃₄" "₅₆₇₈₉" "½¼¾ø∞"
                 "⁰¹²³⁴" "⁵⁶⁷⁸⁹" "⅛⅜⅝⅞|"
                 "≤≠≈≡≥" "√∛∜ℎℏ" "⌐¬⌠⌡−"
                 "∩∟∠∡⊿" "⨯⋅⊕⊖"),
    "Punct",    ".,;:!?" "#$%&'\"" "¡¿`´~\\",
    "Delim",    "()[]{}" "«»'\"¦§" "¨­¯ ’\t",
    "Greek",    ("αβγδεΑΒΓΔΕάΆϵΈέζηθικΖΗΘΙΚΉήϊίΊλμνξοΛΜΝΞΟʹ͵΅Όό"
                 "πρστυΠΡΣΤΥ·ϋςΎύφχψωΰΦΧΨΩ΄ϕ;ϖώΏ"
                 "\n"
                 "aαά bβ gγ dδ eεέϵ AΑΆ BΒ GΓ DΔ EΕΈ "
                 "zζ hηή qθ iιίϊ kκ ZΖ HΗHΉ QΘ IΙΊ KΚ "
                 "lλ mμ nν xξ oοό LΛ MΜ NΝ XΞ OΟΌ "
                 "pπ rρ sσς tτ uυύϋΰ PΠ RΡ SΣ TΤ UΥΎ "
                 "fφϕ cχ vψ wωώϖ FΦ CΧ VΨ WΩΏ"),

    "Arrows",   ("←↑→↓↔"
                 "↕⇄⇆↨⌂"
                 "▲▼◀▬▶"
                 "◢◣◄▪►"
                 "◥◤◀■▶"),
    "Blocks",   ("┌┬┐─"
                 "├┼┤│"
                 "└┴┘▬"
                 "╒╤╕▄"
                 "╞╪╡█"
                 "╘╧╛▀"
                 "╓╥╖▌"
                 "╟╫╢▐"
                 "╙╨╜▪"
                 "╔╦╗═"
                 "╠╬╣║"
                 "╚╩╝■ "
                 "░▒▓□▫"
                 "   ▫▪"),

    "Bullets",  ("·∙►▶→"
                 "■□▪▫▬"
                 "○●◊◘◙"),
    "Money", "$€¢£¤¥₣₤₧₫₭₹₺₽ƒ",
    "Europe",   ("ÀÁÂÃÄ"
                 "àáâãä"
                 "ÅÆÇ"
                 "åæç"
                 "ÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞß"
                 "èéêëìíîïðñòóôõöøùúûüýþÿ"
                 "ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģ"
                 "ĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋ"
                 "ŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲų"
                 "ŴŵŶŷŸŹźŻżŽžſƒƠơƯưǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǺǻǼǽǾǿ"
                 "ȘșȚțȷ"),
    "Cyrillic", ("АБВГДабвгд     "
                 "ЕЖЗИЙежзий     "
                 "КЛМНОклмно     "
                 "ПРСТУпрсту     "
                 "ФХЦЧШфхцчш     "
                 "ЩЪЫЬЭщъыьэ     "
                 "ЮЯ   юя        "
                 "\n"
                 "AАЯ BБ VВ GГҐЃ DДЂ "
                 "aая bб vв gг dд "
                 "EЭЕ ZЖЗЅ IИІ "
                 "eэе zжзѕ iиі "
                 "JЈ KКЌ LЛЉ MМ NНЊ OО "
                 "jј kкќ lлљ mм nнњ oо "
                 "PП RР SСЋ TТ UУЮ "
                 "pп rр sсћ tт uую "
                 "FФ KХ HЦ XЧ YЏ WШ CЩ "
                 "fф kх hц xч yџ wш cщ "
                 "QЪЬЫ"
                 "qъьы"),

    "Picto",    ("⌂№℡™⚙"
                 "☺☻☼♀♂"
                 "♠♣♥♦◊"
                 "♪♫○●▬"),
    "Music",    "♩♪♫♭♮♯",
    "XNum",    ("⁰¹²³⁴"
                 "₀₁₂₃₄"
                 "ⅠⅡⅢⅣⅤ"
                 "⁵⁶⁷⁸⁹"
                 "₅₆₇₈₉"
                 "ⅥⅦⅧⅨⅩ"
                 "ⅪⅫⅬⅭⅮ"
                 "Ⅿ⅛⅜⅝⅞"
                 "⁳№⁻"),
    "XLttr",    "$&@¢©¥ℂ℅ℊℎℏℓ№ℚℝ℡™Å℮ℼⅈⅉⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫⅬⅭⅮⅯ",

    "All",      (" !\"#$%&'()*+,-./0123456789:;<=>?@"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
                 "abcdefghijklmnopqrstuvwxyz{|}~"
                 " ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿"
                 "ⒸⒹⒺⓁⓅⓧ"
                 "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæ"
                 "çèéêëìíîïðñòóôõö÷øùúûüýþÿĀ"
                 "āĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħ"
                 "ĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁł"
                 "ŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠš"
                 "ŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ"
                 "ƒƠơƯưǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǺǻǼǽǾǿȘșȚțȷ"
                 "ʳˀˆˇˉ˘˙˚˛˜˝̣ʹ͵;"
                 "΄Ά·ΈΉΊΌΎΏΐΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩΪΫ"
                 "άέήίΰαβγδεζηθικλμνξοπρςστυφχψωϊϋόύώϕϖ"
                 "ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏ"
                 "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
                 "абвгдежзийклмнопрстуфхцчшщъыьэюя"
                 "ѐёђѓєѕіїјљњћќѝўџҐґ"
                 "ẀẁẂẃẄẅẞẠạẢảẤấẦầẨẩẪẫẬậẮắẰằẲẳẴẵẶặẸẹẺẻẼẽẾếỀ"
                 "ềỂểỄễỆệỈỉỊịỌọỎỏỐốỒồỔổỖỗỘộỚớỜờỞởỠỡỢợỤụỦủỨứỪừỬửỮữỰựỲỳỴỵỶỷỸỹ"
                 "      ‐–—―‗‘’‚‛“”„†‡•…‰′″‹›‼‽‾"
                 "⁄ ⁰⁳⁴⁵⁶⁷⁸⁹⁻ⁿ₀₁₂₃₄₅₆₇₈₉₣₤₧₫€₭₹₺₽"
                 "ℂ℅ℊℎℏℓ№ℚℝ℡™ΩΩÅ℮ℼⅈⅉ⅛⅜⅝⅞"
                 "ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫⅬⅭⅮⅯ"
                 "←↑→↓↔↕↨⇄⇆∂∆∏∑−∕∙√∛∜∞∟∠∡∩∫≈≠≡≤≥⊿⌂⌐⌠⌡"
                 "─│┌┐└┘├┤┬┴┼═║╒╓╔╕╖╗╘╙╚╛╜╝╞╟╠╡╢╣╤╥╦╧╨╩╪╫╬"
                 "▀▄█▌▐░▒▓■□▪▫▬▲▶►▼◀◄◊"
                 "○●◘◙◢◣◤◥◦☺☻☼♀♂♠♣♥♦♪♫⚙")
};
//   clang-format on





// ============================================================================
//
//   Build a characters menu
//
// ============================================================================

symbol_p character_menu::characters(size_t *matchptr) const
// ----------------------------------------------------------------------------
//   Return the character list (value for the menu entry)
// ----------------------------------------------------------------------------
{
    // Use the characters loaded from the characters file
    characters_file cfile(CFILE);
    size_t   matching = 0;
    size_t   maxu   = sizeof(basic_characters) / sizeof(basic_characters[0]);
    id       type   = this->type();
    id       menu   = ID_CharactersMenu00;
    symbol_g mchars = nullptr;

    if (cfile.valid())
    {
        while (text_p mname = cfile.next())
        {
            mchars = cfile.next();
            if (mchars && mname->length())
            {
                if (menu == type)
                {
                    size_t len = 0;
                    utf8   val = mchars->value(&len);
                    utf8   end = val + len;
                    for (utf8 p = val; p < end && *p >= ' '; p = utf8_next(p))
                        matching++;
                    menu = id(menu + 1);
                    break;
                }
                menu = id(menu + 1);
            }
        }
    }

    // Disable built-in characters if we loaded a file
    if (!matching || Settings.ShowBuiltinCharacters())
    {
        for (size_t u = 0; u < maxu; u += 2)
        {
            if (basic_characters[u] && *basic_characters[u])
            {
                if (menu == type)
                {
                    utf8   mtxt = utf8(basic_characters[u + 1]);
                    size_t len  = strlen(cstring(mtxt));
                    mchars      = symbol::make(mtxt, len);
                    for (utf8 p = mtxt; *p && *p >= ' '; p = utf8_next(p))
                        matching++;
                    break;
                }
                menu = id(menu + 1);
            }
        }
    }

    if (matchptr)
        *matchptr = matching;

    return mchars;
}


MENU_BODY(character_menu)
// ----------------------------------------------------------------------------
//   Build a characters menu
// ----------------------------------------------------------------------------
{
    size_t   matching = 0;
    symbol_g mchars   = o->characters(&matching);

    items_init(mi, matching);

    utf8 next = nullptr;
    if (mchars)
    {
        for (utf8 p = mchars->value(); matching--; p = next)
        {
            next = utf8_next(p);
            symbol_g label = symbol::make(p, size_t(next - p));
            items(mi, label, ID_SelfInsert);
        }
    }

    return true;
}


MENU_BODY(CharactersMenu)
// ----------------------------------------------------------------------------
//   The characters menu is dynamically populated
// ----------------------------------------------------------------------------
{
    character_menu::build_general_menu(mi);
    return true;
}


uint character_menu::build_general_menu(menu_info &mi)
// ----------------------------------------------------------------------------
//   Build a menu displaying the various possible classes of character
// ----------------------------------------------------------------------------
{
    uint   infile   = 0;
    uint   count    = 0;
    uint   maxmenus = ID_CharactersMenu99 - ID_CharactersMenu00;
    size_t maxu     = sizeof(basic_characters) / sizeof(basic_characters[0]);
    characters_file cfile(CFILE);

    // List all menu entries in the file (up to 100)
    if (cfile.valid())
        while (symbol_g mname = cfile.next())
            if (symbol_g mvalue = cfile.next())
                if (mname->length())
                    if (infile++ >= maxmenus)
                        break;

    // Count built-in character menu titles
    if (!infile || Settings.ShowBuiltinCharacters())
    {
        for (size_t u = 0; u < maxu; u += 2)
            if (basic_characters[u] && *basic_characters[u])
                count++;
        if (infile + count > maxmenus)
            count = maxmenus - infile;
    }

    items_init(mi, infile + count);
    infile = 0;
    if (cfile.valid())
    {
        cfile.seek(0);
        while (symbol_g mname = cfile.next())
        {
            if (symbol_g mvalue = cfile.next())
            {
                if (mname && mname->length())
                {
                    if (infile >= maxmenus)
                        break;
                    items(mi, mname, id(ID_CharactersMenu00 + infile++));
                }
            }
        }
    }
    if (!infile || Settings.ShowBuiltinCharacters())
    {
        for (size_t u = 0; u < maxu; u += 2)
        {
            if (basic_characters[u] && *basic_characters[u])
            {
                if (infile >= maxmenus)
                    break;
                items(mi, basic_characters[u],
                      id(ID_CharactersMenu00+infile++));
            }
        }
    }

    return true;
}


uint character_menu::build_at_cursor(menu_info &mi)
// ----------------------------------------------------------------------------
//   Build character catalog for cursor position
// ----------------------------------------------------------------------------
{
    unicode cp = ui.character_left_of_cursor();
    return build_for_code(mi, cp);
}


uint character_menu::build_for_code(menu_info &mi, unicode cp)
// ----------------------------------------------------------------------------
//   Build character catalog for a given code point
// ----------------------------------------------------------------------------
{
    // Use the characters loaded from the characters file
    characters_file cfile(CFILE);
    size_t   maxu      = sizeof(basic_characters) / sizeof(basic_characters[0]);
    symbol_g menuchars = nullptr;
    uint     offset    = 0;

    if (cfile.valid())
    {
        while (text_p mname = cfile.next())
        {
            symbol_g mchars = cfile.next();
            if (mchars && !mname->length())
            {
                if (cp)
                {
                    size_t len = 0;
                    utf8 val = mchars->value(&len);
                    utf8 end = val + len;
                    uint found = ~0U;
                    for (utf8 p = val; !~found && p < end; p = utf8_next(p))
                        if (utf8_codepoint(p) == cp)
                            found = p - val;
                    if (~found)
                    {
                        if (!menuchars)
                            offset = found;
                        menuchars = menuchars + mchars;
                    }
                }
                else
                {
                    menuchars = menuchars + mchars;
                }
            }
        }
    }

     // Disable built-in characters if we loaded a file
    if (!menuchars || Settings.ShowBuiltinCharacters())
    {
        for (size_t u = 0; u < maxu; u += 2)
        {
            if (!basic_characters[u] || !*basic_characters[u])
            {
                utf8 mtxt = utf8(basic_characters[u + 1]);
                if (cp)
                {
                    uint found = ~0U;
                    for (utf8 p = mtxt; !~found && *p; p = utf8_next(p))
                        if (utf8_codepoint(p) == cp)
                            found = p - mtxt;
                    if (~found)
                    {
                        if (!menuchars)
                            offset = found;
                        symbol_g mchars = symbol::make(cstring(mtxt));
                        menuchars = menuchars + mchars;
                    }
                }
                else
                {
                    symbol_g mchars = symbol::make(cstring(mtxt));
                    menuchars = menuchars + mchars;
                }
            }
        }
    }

    cfile.close();
    if (menuchars)
    {
        size_t len   = 0;
        utf8   txt   = menuchars->value(&len);
        return build_from_characters(mi, txt, len, offset);
    }
    return build_general_menu(mi);
}


uint character_menu::build_from_characters(menu_info &mi,
                                           utf8 txt, size_t len,
                                           size_t offset)
// ----------------------------------------------------------------------------
//   Build a character menu from the given characters
// ----------------------------------------------------------------------------
{
    utf8   end   = txt + len;
    uint   count = 0;
    uint   index = 0;
    for (utf8 p = txt; p < end; p = utf8_next(p))
    {
        count++;
        if (p == txt + offset)
            index = count;
    }
    if (count)
    {
        items_init(mi, count);
        index = (index + count - 3) % count;
        for (utf8 p = txt; p < end; p = utf8_next(p))
        {
            if (index-- == 0)
            {
                offset = p - txt;
                break;
            }
        }

        utf8 next = nullptr;
        for (utf8 p = txt + offset; count--; p = next)
        {
            next = utf8_next(p);
            symbol_g label = symbol::make(p, size_t(next -p));
            items(mi, label, ID_ReplaceChar);
            if (next >= end)
                next = txt;
        }
        return count;
    }

    return 0;
}


bool character_menu::transliterate(unicode &input) const
// ----------------------------------------------------------------------------
//   Transliterate input if applicable, return true if left of cursor
// ----------------------------------------------------------------------------
{
    size_t   matching = 0;
    symbol_g mchars   = characters(&matching);

    if (mchars)
    {
        size_t  len = 0;
        utf8    txt = mchars->value(&len);
        utf8    end = txt + len;
        unicode now = ui.character_left_of_cursor();

        utf8 tmap = txt;
        while (tmap < end && *tmap != '\n')
            tmap = utf8_next(tmap);
        if (*tmap != '\n')
            return 0;
        tmap++;

        while (tmap < end)
        {
            unicode base = utf8_codepoint(tmap);
            tmap = utf8_next(tmap);
            if (base == input)
            {
                bool hadit = input == now;
                unicode xlate = utf8_codepoint(tmap);
                bool    match = false;
                while (tmap < end)
                {
                    unicode cp = utf8_codepoint(tmap);
                    tmap = utf8_next(tmap);
                    if (cp == ' ')
                        break;
                    if (match)
                    {
                        input = cp;
                        return true;
                    }
                    if (cp == now)
                        match = true;
                }
                if (match)
                    return true;
                input = xlate;
                return hadit;
            }
            else
            {
                while (tmap < end)
                {
                    unicode cp = utf8_codepoint(tmap);
                    tmap = utf8_next(tmap);
                    if (cp == ' ')
                        break;
                }
            }
        }
    }
    return false;
}
