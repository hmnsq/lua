/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <locale.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lctype.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "llex.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lparser.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "lzio.h"



#define পরবর্তী(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#যদি !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
স্থির ধ্রুবক অক্ষর *ধ্রুবক luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_পরবর্তী(ls) (save(ls, ls->current), পরবর্তী(ls))


স্থির l_noret lexerror (LexState *ls, ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা token);


স্থির শূন্য save (LexState *ls, পূর্ণসংখ্যা c) {
  Mbuffer *b = ls->buff;
  যদি (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    যদি (newsize >= (MAX_আকার/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->বাফার[luaZ_bufflen(b)++] = cast_char(c);
}


শূন্য luaX_init (lua_State *L) {
  পূর্ণসংখ্যা i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  জন্য (i=0; i<অঙ্ক_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ধ্রুবক অক্ষর *luaX_token2str (LexState *ls, পূর্ণসংখ্যা token) {
  যদি (token < FIRST_RESERVED) {  /* single-byte symbols? */
    যদি (lisprint(token))
      ফেরত luaO_pushfstring(ls->L, "'%c'", token);
    অন্যথা  /* control character */
      ফেরত luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  অন্যথা {
    ধ্রুবক অক্ষর *s = luaX_tokens[token - FIRST_RESERVED];
    যদি (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      ফেরত luaO_pushfstring(ls->L, "'%s'", s);
    অন্যথা  /* names, strings, and numerals */
      ফেরত s;
  }
}


স্থির ধ্রুবক অক্ষর *txtToken (LexState *ls, পূর্ণসংখ্যা token) {
  নির্বাচন (token) {
    ক্ষেত্র TK_NAME: ক্ষেত্র TK_STRING:
    ক্ষেত্র TK_FLT: ক্ষেত্র TK_INT:
      save(ls, '\0');
      ফেরত luaO_pushfstring(ls->L, "'%s'", lua_Z_বাফার(ls->buff));
    পূর্বনির্ধারিত:
      ফেরত luaX_token2str(ls, token);
  }
}


স্থির l_noret lexerror (LexState *ls, ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা token) {
  বার্তা = luaG_addinfo(ls->L, বার্তা, ls->source, ls->linenumber);
  যদি (token)
    luaO_pushfstring(ls->L, "%s near %s", বার্তা, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ধ্রুবক অক্ষর *বার্তা) {
  lexerror(ls, বার্তা, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
স্থির TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  পূর্ণসংখ্যা tag = luaH_getstr(ls->h, ts, &oldts);
  যদি (!tagisempty(tag))  /* string already present? */
    ফেরত tsvalue(&oldts);  /* use stored value */
  অন্যথা {  /* create a new entry */
    TValue *stv = s2v(L->শীর্ষ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->শীর্ষ.p--;  /* remove string from stack */
    ফেরত ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ধ্রুবক অক্ষর *str, size_t l) {
  ফেরত anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
স্থির শূন্য inclinenumber (LexState *ls) {
  পূর্ণসংখ্যা old = ls->current;
  lua_assert(currIsNewline(ls));
  পরবর্তী(ls);  /* skip '\n' or '\r' */
  যদি (currIsNewline(ls) && ls->current != old)
    পরবর্তী(ls);  /* skip '\n\r' or '\r\n' */
  যদি (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


শূন্য luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    পূর্ণসংখ্যা firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ফাঁকা;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#যদি LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#endif
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


স্থির পূর্ণসংখ্যা check_next1 (LexState *ls, পূর্ণসংখ্যা c) {
  যদি (ls->current == c) {
    পরবর্তী(ls);
    ফেরত 1;
  }
  অন্যথা ফেরত 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
স্থির পূর্ণসংখ্যা check_next2 (LexState *ls, ধ্রুবক অক্ষর *set) {
  lua_assert(set[2] == '\0');
  যদি (ls->current == set[0] || ls->current == set[1]) {
    save_and_পরবর্তী(ls);
    ফেরত 1;
  }
  অন্যথা ফেরত 0;
}


/* LUA_NUMBER */
/*
** This function is quite liberal in what it accepts, as 'luaO_str2num'
** will reject ill-formed numerals. Roughly, it accepts the following
** pattern:
**
**   %d(%x|%.|([Ee][+-]?))* | 0[Xx](%x|%.|([Pp][+-]?))*
**
** The only tricky part is to accept [+-] only after a valid exponent
** mark, to avoid reading '3-4' or '0xe+1' as a single number.
**
** The caller might have already read an initial dot.
*/
স্থির পূর্ণসংখ্যা read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ধ্রুবক অক্ষর *expo = "Ee";
  পূর্ণসংখ্যা first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_পরবর্তী(ls);
  যদি (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  জন্য (;;) {
    যদি (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    অন্যথা যদি (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_পরবর্তী(ls);
    অন্যথা বিরতি;
  }
  যদি (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_পরবর্তী(ls);  /* force an error */
  save(ls, '\0');
  যদি (luaO_str2num(lua_Z_বাফার(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  যদি (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    ফেরত TK_INT;
  }
  অন্যথা {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    ফেরত TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
স্থির size_t skip_sep (LexState *ls) {
  size_t গুনতি = 0;
  পূর্ণসংখ্যা s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_পরবর্তী(ls);
  যখন (ls->current == '=') {
    save_and_পরবর্তী(ls);
    গুনতি++;
  }
  ফেরত (ls->current == s) ? গুনতি + 2
         : (গুনতি == 0) ? 1
         : 0;
}


স্থির শূন্য read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  পূর্ণসংখ্যা সারি = ls->linenumber;  /* initial line (for error message) */
  save_and_পরবর্তী(ls);  /* skip 2nd '[' */
  যদি (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  জন্য (;;) {
    নির্বাচন (ls->current) {
      ক্ষেত্র EOZ: {  /* error */
        ধ্রুবক অক্ষর *what = (seminfo ? "string" : "comment");
        ধ্রুবক অক্ষর *বার্তা = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, সারি);
        lexerror(ls, বার্তা, TK_EOS);
        বিরতি;  /* to avoid warnings */
      }
      ক্ষেত্র ']': {
        যদি (skip_sep(ls) == sep) {
          save_and_পরবর্তী(ls);  /* skip 2nd ']' */
          যাও endloop;
        }
        বিরতি;
      }
      ক্ষেত্র '\n': ক্ষেত্র '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        যদি (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        বিরতি;
      }
      পূর্বনির্ধারিত: {
        যদি (seminfo) save_and_পরবর্তী(ls);
        অন্যথা পরবর্তী(ls);
      }
    }
  } endloop:
  যদি (seminfo)
    seminfo->ts = luaX_newstring(ls, lua_Z_বাফার(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


স্থির শূন্য esccheck (LexState *ls, পূর্ণসংখ্যা c, ধ্রুবক অক্ষর *বার্তা) {
  যদি (!c) {
    যদি (ls->current != EOZ)
      save_and_পরবর্তী(ls);  /* add current to buffer for error message */
    lexerror(ls, বার্তা, TK_STRING);
  }
}


স্থির পূর্ণসংখ্যা gethexa (LexState *ls) {
  save_and_পরবর্তী(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  ফেরত luaO_hexavalue(ls->current);
}


স্থির পূর্ণসংখ্যা readhexaesc (LexState *ls) {
  পূর্ণসংখ্যা r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  ফেরত r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
স্থির l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  পূর্ণসংখ্যা i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_পরবর্তী(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  যখন (cast_void(save_and_পরবর্তী(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  পরবর্তী(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  ফেরত r;
}


স্থির শূন্য utf8esc (LexState *ls) {
  অক্ষর buff[UTF8BUFFSZ];
  পূর্ণসংখ্যা n = luaO_utf8esc(buff, readutf8esc(ls));
  জন্য (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


স্থির পূর্ণসংখ্যা readdecesc (LexState *ls) {
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা r = 0;  /* result accumulator */
  জন্য (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_পরবর্তী(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  ফেরত r;
}


স্থির শূন্য read_string (LexState *ls, পূর্ণসংখ্যা del, SemInfo *seminfo) {
  save_and_পরবর্তী(ls);  /* keep delimiter (for error messages) */
  যখন (ls->current != del) {
    নির্বাচন (ls->current) {
      ক্ষেত্র EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        বিরতি;  /* to avoid warnings */
      ক্ষেত্র '\n':
      ক্ষেত্র '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        বিরতি;  /* to avoid warnings */
      ক্ষেত্র '\\': {  /* escape sequences */
        পূর্ণসংখ্যা c;  /* final character to be saved */
        save_and_পরবর্তী(ls);  /* keep '\\' for error messages */
        নির্বাচন (ls->current) {
          ক্ষেত্র 'a': c = '\a'; যাও read_save;
          ক্ষেত্র 'b': c = '\b'; যাও read_save;
          ক্ষেত্র 'f': c = '\f'; যাও read_save;
          ক্ষেত্র 'n': c = '\n'; যাও read_save;
          ক্ষেত্র 'r': c = '\r'; যাও read_save;
          ক্ষেত্র 't': c = '\t'; যাও read_save;
          ক্ষেত্র 'v': c = '\v'; যাও read_save;
          ক্ষেত্র 'x': c = readhexaesc(ls); যাও read_save;
          ক্ষেত্র 'u': utf8esc(ls);  যাও no_save;
          ক্ষেত্র '\n': ক্ষেত্র '\r':
            inclinenumber(ls); c = '\n'; যাও only_save;
          ক্ষেত্র '\\': ক্ষেত্র '\"': ক্ষেত্র '\'':
            c = ls->current; যাও read_save;
          ক্ষেত্র EOZ: যাও no_save;  /* will raise an error next loop */
          ক্ষেত্র 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            পরবর্তী(ls);  /* skip the 'z' */
            যখন (lisspace(ls->current)) {
              যদি (currIsNewline(ls)) inclinenumber(ls);
              অন্যথা পরবর্তী(ls);
            }
            যাও no_save;
          }
          পূর্বনির্ধারিত: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            যাও only_save;
          }
        }
       read_save:
         পরবর্তী(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: বিরতি;
      }
      পূর্বনির্ধারিত:
        save_and_পরবর্তী(ls);
    }
  }
  save_and_পরবর্তী(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, lua_Z_বাফার(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


স্থির পূর্ণসংখ্যা llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  জন্য (;;) {
    নির্বাচন (ls->current) {
      ক্ষেত্র '\n': ক্ষেত্র '\r': {  /* line breaks */
        inclinenumber(ls);
        বিরতি;
      }
      ক্ষেত্র ' ': ক্ষেত্র '\f': ক্ষেত্র '\t': ক্ষেত্র '\v': {  /* spaces */
        পরবর্তী(ls);
        বিরতি;
      }
      ক্ষেত্র '-': {  /* '-' or '--' (comment) */
        পরবর্তী(ls);
        যদি (ls->current != '-') ফেরত '-';
        /* else is a comment */
        পরবর্তী(ls);
        যদি (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          যদি (sep >= 2) {
            read_long_string(ls, ফাঁকা, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            বিরতি;
          }
        }
        /* else short comment */
        যখন (!currIsNewline(ls) && ls->current != EOZ)
          পরবর্তী(ls);  /* skip until end of line (or end of file) */
        বিরতি;
      }
      ক্ষেত্র '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        যদি (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          ফেরত TK_STRING;
        }
        অন্যথা যদি (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        ফেরত '[';
      }
      ক্ষেত্র '=': {
        পরবর্তী(ls);
        যদি (check_next1(ls, '=')) ফেরত TK_EQ;  /* '==' */
        অন্যথা ফেরত '=';
      }
      ক্ষেত্র '<': {
        পরবর্তী(ls);
        যদি (check_next1(ls, '=')) ফেরত TK_LE;  /* '<=' */
        অন্যথা যদি (check_next1(ls, '<')) ফেরত TK_SHL;  /* '<<' */
        অন্যথা ফেরত '<';
      }
      ক্ষেত্র '>': {
        পরবর্তী(ls);
        যদি (check_next1(ls, '=')) ফেরত TK_GE;  /* '>=' */
        অন্যথা যদি (check_next1(ls, '>')) ফেরত TK_SHR;  /* '>>' */
        অন্যথা ফেরত '>';
      }
      ক্ষেত্র '/': {
        পরবর্তী(ls);
        যদি (check_next1(ls, '/')) ফেরত TK_IDIV;  /* '//' */
        অন্যথা ফেরত '/';
      }
      ক্ষেত্র '~': {
        পরবর্তী(ls);
        যদি (check_next1(ls, '=')) ফেরত TK_NE;  /* '~=' */
        অন্যথা ফেরত '~';
      }
      ক্ষেত্র ':': {
        পরবর্তী(ls);
        যদি (check_next1(ls, ':')) ফেরত TK_DBCOLON;  /* '::' */
        অন্যথা ফেরত ':';
      }
      ক্ষেত্র '"': ক্ষেত্র '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        ফেরত TK_STRING;
      }
      ক্ষেত্র '.': {  /* '.', '..', '...', or number */
        save_and_পরবর্তী(ls);
        যদি (check_next1(ls, '.')) {
          যদি (check_next1(ls, '.'))
            ফেরত TK_DOTS;   /* '...' */
          অন্যথা ফেরত TK_CONCAT;   /* '..' */
        }
        অন্যথা যদি (!lisdigit(ls->current)) ফেরত '.';
        অন্যথা ফেরত read_numeral(ls, seminfo);
      }
      ক্ষেত্র '0': ক্ষেত্র '1': ক্ষেত্র '2': ক্ষেত্র '3': ক্ষেত্র '4':
      ক্ষেত্র '5': ক্ষেত্র '6': ক্ষেত্র '7': ক্ষেত্র '8': ক্ষেত্র '9': {
        ফেরত read_numeral(ls, seminfo);
      }
      ক্ষেত্র EOZ: {
        ফেরত TK_EOS;
      }
      পূর্বনির্ধারিত: {
        যদি (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          করো {
            save_and_পরবর্তী(ls);
          } যখন (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, lua_Z_বাফার(ls->buff),
                                   luaZ_bufflen(ls->buff));
          যদি (isreserved(ts))   /* reserved word? */
            ফেরত ts->extra - 1 + FIRST_RESERVED;
          অন্যথা {
            seminfo->ts = anchorstr(ls, ts);
            ফেরত TK_NAME;
          }
        }
        অন্যথা {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          পূর্ণসংখ্যা c = ls->current;
          পরবর্তী(ls);
          ফেরত c;
        }
      }
    }
  }
}


শূন্য lua_X_পরবর্তী (LexState *ls) {
  ls->lastline = ls->linenumber;
  যদি (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  অন্যথা
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


পূর্ণসংখ্যা luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  ফেরত ls->lookahead.token;
}

