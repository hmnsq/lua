/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lctype.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "llex.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lparser.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#എങ്കിൽ !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


സ്ഥിതം l_noret lexerror (LexState *ls, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ token);


സ്ഥിതം ശൂന്യം save (LexState *ls, പൂർണ്ണസംഖ്യ c) {
  Mbuffer *b = ls->buff;
  എങ്കിൽ (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    എങ്കിൽ (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


ശൂന്യം luaX_init (lua_State *L) {
  പൂർണ്ണസംഖ്യ i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  വേണ്ടി (i=0; i<അക്കം_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


സ്ഥിരം അക്ഷരം *luaX_token2str (LexState *ls, പൂർണ്ണസംഖ്യ token) {
  എങ്കിൽ (token < FIRST_RESERVED) {  /* single-byte symbols? */
    എങ്കിൽ (lisprint(token))
      തിരികെ luaO_pushfstring(ls->L, "'%c'", token);
    അല്ലെങ്കിൽ  /* control character */
      തിരികെ luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *s = luaX_tokens[token - FIRST_RESERVED];
    എങ്കിൽ (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      തിരികെ luaO_pushfstring(ls->L, "'%s'", s);
    അല്ലെങ്കിൽ  /* names, strings, and numerals */
      തിരികെ s;
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *txtToken (LexState *ls, പൂർണ്ണസംഖ്യ token) {
  തിരഞ്ഞെടുപ്പ് (token) {
    അവസ്ഥ TK_NAME: അവസ്ഥ TK_STRING:
    അവസ്ഥ TK_FLT: അവസ്ഥ TK_INT:
      save(ls, '\0');
      തിരികെ luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    സ്ഥിരസ്ഥിതി:
      തിരികെ luaX_token2str(ls, token);
  }
}


സ്ഥിതം l_noret lexerror (LexState *ls, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ token) {
  സന്ദേശം = luaG_addinfo(ls->L, സന്ദേശം, ls->source, ls->linenumber);
  എങ്കിൽ (token)
    luaO_pushfstring(ls->L, "%s near %s", സന്ദേശം, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
  lexerror(ls, സന്ദേശം, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
സ്ഥിതം TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  പൂർണ്ണസംഖ്യ tag = luaH_getstr(ls->h, ts, &oldts);
  എങ്കിൽ (!tagisempty(tag))  /* string already present? */
    തിരികെ tsvalue(&oldts);  /* use stored value */
  അല്ലെങ്കിൽ {  /* create a new entry */
    TValue *stv = s2v(L->മുകൾ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->മുകൾ.p--;  /* remove string from stack */
    തിരികെ ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, സ്ഥിരം അക്ഷരം *str, size_t l) {
  തിരികെ anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
സ്ഥിതം ശൂന്യം inclinenumber (LexState *ls) {
  പൂർണ്ണസംഖ്യ old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  എങ്കിൽ (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  എങ്കിൽ (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


ശൂന്യം luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    പൂർണ്ണസംഖ്യ firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ശൂന്യ;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#എങ്കിൽ LUA_COMPAT_GLOBAL
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


സ്ഥിതം പൂർണ്ണസംഖ്യ check_next1 (LexState *ls, പൂർണ്ണസംഖ്യ c) {
  എങ്കിൽ (ls->current == c) {
    next(ls);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ തിരികെ 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ check_next2 (LexState *ls, സ്ഥിരം അക്ഷരം *set) {
  lua_assert(set[2] == '\0');
  എങ്കിൽ (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  സ്ഥിരം അക്ഷരം *expo = "Ee";
  പൂർണ്ണസംഖ്യ first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  എങ്കിൽ (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  വേണ്ടി (;;) {
    എങ്കിൽ (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    അല്ലെങ്കിൽ എങ്കിൽ (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    അല്ലെങ്കിൽ നിർത്തുക;
  }
  എങ്കിൽ (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  എങ്കിൽ (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  എങ്കിൽ (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    തിരികെ TK_INT;
  }
  അല്ലെങ്കിൽ {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    തിരികെ TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
സ്ഥിതം size_t skip_sep (LexState *ls) {
  size_t എണ്ണം = 0;
  പൂർണ്ണസംഖ്യ s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  ആകുമ്പോൾ (ls->current == '=') {
    save_and_next(ls);
    എണ്ണം++;
  }
  തിരികെ (ls->current == s) ? എണ്ണം + 2
         : (എണ്ണം == 0) ? 1
         : 0;
}


സ്ഥിതം ശൂന്യം read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  പൂർണ്ണസംഖ്യ വരി = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  എങ്കിൽ (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  വേണ്ടി (;;) {
    തിരഞ്ഞെടുപ്പ് (ls->current) {
      അവസ്ഥ EOZ: {  /* error */
        സ്ഥിരം അക്ഷരം *what = (seminfo ? "string" : "comment");
        സ്ഥിരം അക്ഷരം *സന്ദേശം = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, വരി);
        lexerror(ls, സന്ദേശം, TK_EOS);
        നിർത്തുക;  /* to avoid warnings */
      }
      അവസ്ഥ ']': {
        എങ്കിൽ (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          പോകുക endloop;
        }
        നിർത്തുക;
      }
      അവസ്ഥ '\n': അവസ്ഥ '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        എങ്കിൽ (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: {
        എങ്കിൽ (seminfo) save_and_next(ls);
        അല്ലെങ്കിൽ next(ls);
      }
    }
  } endloop:
  എങ്കിൽ (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


സ്ഥിതം ശൂന്യം esccheck (LexState *ls, പൂർണ്ണസംഖ്യ c, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
  എങ്കിൽ (!c) {
    എങ്കിൽ (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, സന്ദേശം, TK_STRING);
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  തിരികെ luaO_hexavalue(ls->current);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ readhexaesc (LexState *ls) {
  പൂർണ്ണസംഖ്യ r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  തിരികെ r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
സ്ഥിതം l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  പൂർണ്ണസംഖ്യ i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  ആകുമ്പോൾ (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  തിരികെ r;
}


സ്ഥിതം ശൂന്യം utf8esc (LexState *ls) {
  അക്ഷരം buff[UTF8BUFFSZ];
  പൂർണ്ണസംഖ്യ n = luaO_utf8esc(buff, readutf8esc(ls));
  വേണ്ടി (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ readdecesc (LexState *ls) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ r = 0;  /* result accumulator */
  വേണ്ടി (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  തിരികെ r;
}


സ്ഥിതം ശൂന്യം read_string (LexState *ls, പൂർണ്ണസംഖ്യ del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  ആകുമ്പോൾ (ls->current != del) {
    തിരഞ്ഞെടുപ്പ് (ls->current) {
      അവസ്ഥ EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        നിർത്തുക;  /* to avoid warnings */
      അവസ്ഥ '\n':
      അവസ്ഥ '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        നിർത്തുക;  /* to avoid warnings */
      അവസ്ഥ '\\': {  /* escape sequences */
        പൂർണ്ണസംഖ്യ c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        തിരഞ്ഞെടുപ്പ് (ls->current) {
          അവസ്ഥ 'a': c = '\a'; പോകുക read_save;
          അവസ്ഥ 'b': c = '\b'; പോകുക read_save;
          അവസ്ഥ 'f': c = '\f'; പോകുക read_save;
          അവസ്ഥ 'n': c = '\n'; പോകുക read_save;
          അവസ്ഥ 'r': c = '\r'; പോകുക read_save;
          അവസ്ഥ 't': c = '\t'; പോകുക read_save;
          അവസ്ഥ 'v': c = '\v'; പോകുക read_save;
          അവസ്ഥ 'x': c = readhexaesc(ls); പോകുക read_save;
          അവസ്ഥ 'u': utf8esc(ls);  പോകുക no_save;
          അവസ്ഥ '\n': അവസ്ഥ '\r':
            inclinenumber(ls); c = '\n'; പോകുക only_save;
          അവസ്ഥ '\\': അവസ്ഥ '\"': അവസ്ഥ '\'':
            c = ls->current; പോകുക read_save;
          അവസ്ഥ EOZ: പോകുക no_save;  /* will raise an error next loop */
          അവസ്ഥ 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            ആകുമ്പോൾ (lisspace(ls->current)) {
              എങ്കിൽ (currIsNewline(ls)) inclinenumber(ls);
              അല്ലെങ്കിൽ next(ls);
            }
            പോകുക no_save;
          }
          സ്ഥിരസ്ഥിതി: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            പോകുക only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  വേണ്ടി (;;) {
    തിരഞ്ഞെടുപ്പ് (ls->current) {
      അവസ്ഥ '\n': അവസ്ഥ '\r': {  /* line breaks */
        inclinenumber(ls);
        നിർത്തുക;
      }
      അവസ്ഥ ' ': അവസ്ഥ '\f': അവസ്ഥ '\t': അവസ്ഥ '\v': {  /* spaces */
        next(ls);
        നിർത്തുക;
      }
      അവസ്ഥ '-': {  /* '-' or '--' (comment) */
        next(ls);
        എങ്കിൽ (ls->current != '-') തിരികെ '-';
        /* else is a comment */
        next(ls);
        എങ്കിൽ (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          എങ്കിൽ (sep >= 2) {
            read_long_string(ls, ശൂന്യ, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            നിർത്തുക;
          }
        }
        /* else short comment */
        ആകുമ്പോൾ (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        നിർത്തുക;
      }
      അവസ്ഥ '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        എങ്കിൽ (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          തിരികെ TK_STRING;
        }
        അല്ലെങ്കിൽ എങ്കിൽ (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        തിരികെ '[';
      }
      അവസ്ഥ '=': {
        next(ls);
        എങ്കിൽ (check_next1(ls, '=')) തിരികെ TK_EQ;  /* '==' */
        അല്ലെങ്കിൽ തിരികെ '=';
      }
      അവസ്ഥ '<': {
        next(ls);
        എങ്കിൽ (check_next1(ls, '=')) തിരികെ TK_LE;  /* '<=' */
        അല്ലെങ്കിൽ എങ്കിൽ (check_next1(ls, '<')) തിരികെ TK_SHL;  /* '<<' */
        അല്ലെങ്കിൽ തിരികെ '<';
      }
      അവസ്ഥ '>': {
        next(ls);
        എങ്കിൽ (check_next1(ls, '=')) തിരികെ TK_GE;  /* '>=' */
        അല്ലെങ്കിൽ എങ്കിൽ (check_next1(ls, '>')) തിരികെ TK_SHR;  /* '>>' */
        അല്ലെങ്കിൽ തിരികെ '>';
      }
      അവസ്ഥ '/': {
        next(ls);
        എങ്കിൽ (check_next1(ls, '/')) തിരികെ TK_IDIV;  /* '//' */
        അല്ലെങ്കിൽ തിരികെ '/';
      }
      അവസ്ഥ '~': {
        next(ls);
        എങ്കിൽ (check_next1(ls, '=')) തിരികെ TK_NE;  /* '~=' */
        അല്ലെങ്കിൽ തിരികെ '~';
      }
      അവസ്ഥ ':': {
        next(ls);
        എങ്കിൽ (check_next1(ls, ':')) തിരികെ TK_DBCOLON;  /* '::' */
        അല്ലെങ്കിൽ തിരികെ ':';
      }
      അവസ്ഥ '"': അവസ്ഥ '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        തിരികെ TK_STRING;
      }
      അവസ്ഥ '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        എങ്കിൽ (check_next1(ls, '.')) {
          എങ്കിൽ (check_next1(ls, '.'))
            തിരികെ TK_DOTS;   /* '...' */
          അല്ലെങ്കിൽ തിരികെ TK_CONCAT;   /* '..' */
        }
        അല്ലെങ്കിൽ എങ്കിൽ (!lisdigit(ls->current)) തിരികെ '.';
        അല്ലെങ്കിൽ തിരികെ read_numeral(ls, seminfo);
      }
      അവസ്ഥ '0': അവസ്ഥ '1': അവസ്ഥ '2': അവസ്ഥ '3': അവസ്ഥ '4':
      അവസ്ഥ '5': അവസ്ഥ '6': അവസ്ഥ '7': അവസ്ഥ '8': അവസ്ഥ '9': {
        തിരികെ read_numeral(ls, seminfo);
      }
      അവസ്ഥ EOZ: {
        തിരികെ TK_EOS;
      }
      സ്ഥിരസ്ഥിതി: {
        എങ്കിൽ (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          ചെയ്യുക {
            save_and_next(ls);
          } ആകുമ്പോൾ (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          എങ്കിൽ (isreserved(ts))   /* reserved word? */
            തിരികെ ts->extra - 1 + FIRST_RESERVED;
          അല്ലെങ്കിൽ {
            seminfo->ts = anchorstr(ls, ts);
            തിരികെ TK_NAME;
          }
        }
        അല്ലെങ്കിൽ {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          പൂർണ്ണസംഖ്യ c = ls->current;
          next(ls);
          തിരികെ c;
        }
      }
    }
  }
}


ശൂന്യം luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  എങ്കിൽ (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  അല്ലെങ്കിൽ
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


പൂർണ്ണസംഖ്യ luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  തിരികെ ls->lookahead.token;
}

