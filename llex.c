/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lctype.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "llex.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lparser.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#ଯଦି !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


ସ୍ଥିର l_noret lexerror (LexState *ls, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା token);


ସ୍ଥିର ଶୂନ୍ୟ save (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  Mbuffer *b = ls->buff;
  ଯଦି (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    ଯଦି (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


ଶୂନ୍ୟ luaX_init (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  ପାଇଁ (i=0; i<ଅଙ୍କ_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ଅଚଳ ଅକ୍ଷର *luaX_token2str (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା token) {
  ଯଦି (token < FIRST_RESERVED) {  /* single-byte symbols? */
    ଯଦି (lisprint(token))
      ଫେରନ୍ତୁ luaO_pushfstring(ls->L, "'%c'", token);
    ଅନ୍ୟଥା  /* control character */
      ଫେରନ୍ତୁ luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *s = luaX_tokens[token - FIRST_RESERVED];
    ଯଦି (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      ଫେରନ୍ତୁ luaO_pushfstring(ls->L, "'%s'", s);
    ଅନ୍ୟଥା  /* names, strings, and numerals */
      ଫେରନ୍ତୁ s;
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *txtToken (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା token) {
  ବାଛ (token) {
    ସ୍ଥିତି TK_NAME: ସ୍ଥିତି TK_STRING:
    ସ୍ଥିତି TK_FLT: ସ୍ଥିତି TK_INT:
      save(ls, '\0');
      ଫେରନ୍ତୁ luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ luaX_token2str(ls, token);
  }
}


ସ୍ଥିର l_noret lexerror (LexState *ls, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା token) {
  ସନ୍ଦେଶ = luaG_addinfo(ls->L, ସନ୍ଦେଶ, ls->source, ls->linenumber);
  ଯଦି (token)
    luaO_pushfstring(ls->L, "%s near %s", ସନ୍ଦେଶ, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
  lexerror(ls, ସନ୍ଦେଶ, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
ସ୍ଥିର TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag = luaH_getstr(ls->h, ts, &oldts);
  ଯଦି (!tagisempty(tag))  /* string already present? */
    ଫେରନ୍ତୁ tsvalue(&oldts);  /* use stored value */
  ଅନ୍ୟଥା {  /* create a new entry */
    TValue *stv = s2v(L->ଶୀର୍ଷ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->ଶୀର୍ଷ.p--;  /* remove string from stack */
    ଫେରନ୍ତୁ ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ଅଚଳ ଅକ୍ଷର *str, size_t l) {
  ଫେରନ୍ତୁ anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
ସ୍ଥିର ଶୂନ୍ୟ inclinenumber (LexState *ls) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  ଯଦି (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  ଯଦି (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


ଶୂନ୍ୟ luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ଖାଲି;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#ଯଦି LUA_COMPAT_GLOBAL
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


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_next1 (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) {
  ଯଦି (ls->current == c) {
    next(ls);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_next2 (LexState *ls, ଅଚଳ ଅକ୍ଷର *set) {
  lua_assert(set[2] == '\0');
  ଯଦି (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ଅଚଳ ଅକ୍ଷର *expo = "Ee";
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  ଯଦି (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  ପାଇଁ (;;) {
    ଯଦି (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    ଅନ୍ୟଥା ଯଦି (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    ଅନ୍ୟଥା ଭାଙ୍ଗ;
  }
  ଯଦି (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  ଯଦି (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  ଯଦି (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    ଫେରନ୍ତୁ TK_INT;
  }
  ଅନ୍ୟଥା {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    ଫେରନ୍ତୁ TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
ସ୍ଥିର size_t skip_sep (LexState *ls) {
  size_t ଗଣନା_2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  ଯେତେବେଳେ (ls->current == '=') {
    save_and_next(ls);
    ଗଣନା_2++;
  }
  ଫେରନ୍ତୁ (ls->current == s) ? ଗଣନା_2 + 2
         : (ଗଣନା_2 == 0) ? 1
         : 0;
}


ସ୍ଥିର ଶୂନ୍ୟ read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  ଯଦି (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  ପାଇଁ (;;) {
    ବାଛ (ls->current) {
      ସ୍ଥିତି EOZ: {  /* error */
        ଅଚଳ ଅକ୍ଷର *what = (seminfo ? "string" : "comment");
        ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, ଧାଡ଼ି);
        lexerror(ls, ସନ୍ଦେଶ, TK_EOS);
        ଭାଙ୍ଗ;  /* to avoid warnings */
      }
      ସ୍ଥିତି ']': {
        ଯଦି (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          ଯାଅ endloop;
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '\n': ସ୍ଥିତି '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        ଯଦି (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ଯଦି (seminfo) save_and_next(ls);
        ଅନ୍ୟଥା next(ls);
      }
    }
  } endloop:
  ଯଦି (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


ସ୍ଥିର ଶୂନ୍ୟ esccheck (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
  ଯଦି (!c) {
    ଯଦି (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, ସନ୍ଦେଶ, TK_STRING);
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  ଫେରନ୍ତୁ luaO_hexavalue(ls->current);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା readhexaesc (LexState *ls) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  ଫେରନ୍ତୁ r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
ସ୍ଥିର l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  ଯେତେବେଳେ (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  ଫେରନ୍ତୁ r;
}


ସ୍ଥିର ଶୂନ୍ୟ utf8esc (LexState *ls) {
  ଅକ୍ଷର buff[UTF8BUFFSZ];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = luaO_utf8esc(buff, readutf8esc(ls));
  ପାଇଁ (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା readdecesc (LexState *ls) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = 0;  /* result accumulator */
  ପାଇଁ (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  ଫେରନ୍ତୁ r;
}


ସ୍ଥିର ଶୂନ୍ୟ read_string (LexState *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  ଯେତେବେଳେ (ls->current != del) {
    ବାଛ (ls->current) {
      ସ୍ଥିତି EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        ଭାଙ୍ଗ;  /* to avoid warnings */
      ସ୍ଥିତି '\n':
      ସ୍ଥିତି '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        ଭାଙ୍ଗ;  /* to avoid warnings */
      ସ୍ଥିତି '\\': {  /* escape sequences */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        ବାଛ (ls->current) {
          ସ୍ଥିତି 'a': c = '\a'; ଯାଅ read_save;
          ସ୍ଥିତି 'b': c = '\b'; ଯାଅ read_save;
          ସ୍ଥିତି 'f': c = '\f'; ଯାଅ read_save;
          ସ୍ଥିତି 'n': c = '\n'; ଯାଅ read_save;
          ସ୍ଥିତି 'r': c = '\r'; ଯାଅ read_save;
          ସ୍ଥିତି 't': c = '\t'; ଯାଅ read_save;
          ସ୍ଥିତି 'v': c = '\v'; ଯାଅ read_save;
          ସ୍ଥିତି 'x': c = readhexaesc(ls); ଯାଅ read_save;
          ସ୍ଥିତି 'u': utf8esc(ls);  ଯାଅ no_save;
          ସ୍ଥିତି '\n': ସ୍ଥିତି '\r':
            inclinenumber(ls); c = '\n'; ଯାଅ only_save;
          ସ୍ଥିତି '\\': ସ୍ଥିତି '\"': ସ୍ଥିତି '\'':
            c = ls->current; ଯାଅ read_save;
          ସ୍ଥିତି EOZ: ଯାଅ no_save;  /* will raise an error next loop */
          ସ୍ଥିତି 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            ଯେତେବେଳେ (lisspace(ls->current)) {
              ଯଦି (currIsNewline(ls)) inclinenumber(ls);
              ଅନ୍ୟଥା next(ls);
            }
            ଯାଅ no_save;
          }
          ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            ଯାଅ only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  ପାଇଁ (;;) {
    ବାଛ (ls->current) {
      ସ୍ଥିତି '\n': ସ୍ଥିତି '\r': {  /* line breaks */
        inclinenumber(ls);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି ' ': ସ୍ଥିତି '\f': ସ୍ଥିତି '\t': ସ୍ଥିତି '\v': {  /* spaces */
        next(ls);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '-': {  /* '-' or '--' (comment) */
        next(ls);
        ଯଦି (ls->current != '-') ଫେରନ୍ତୁ '-';
        /* else is a comment */
        next(ls);
        ଯଦି (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          ଯଦି (sep >= 2) {
            read_long_string(ls, ଖାଲି, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            ଭାଙ୍ଗ;
          }
        }
        /* else short comment */
        ଯେତେବେଳେ (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        ଯଦି (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          ଫେରନ୍ତୁ TK_STRING;
        }
        ଅନ୍ୟଥା ଯଦି (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        ଫେରନ୍ତୁ '[';
      }
      ସ୍ଥିତି '=': {
        next(ls);
        ଯଦି (check_next1(ls, '=')) ଫେରନ୍ତୁ TK_EQ;  /* '==' */
        ଅନ୍ୟଥା ଫେରନ୍ତୁ '=';
      }
      ସ୍ଥିତି '<': {
        next(ls);
        ଯଦି (check_next1(ls, '=')) ଫେରନ୍ତୁ TK_LE;  /* '<=' */
        ଅନ୍ୟଥା ଯଦି (check_next1(ls, '<')) ଫେରନ୍ତୁ TK_SHL;  /* '<<' */
        ଅନ୍ୟଥା ଫେରନ୍ତୁ '<';
      }
      ସ୍ଥିତି '>': {
        next(ls);
        ଯଦି (check_next1(ls, '=')) ଫେରନ୍ତୁ TK_GE;  /* '>=' */
        ଅନ୍ୟଥା ଯଦି (check_next1(ls, '>')) ଫେରନ୍ତୁ TK_SHR;  /* '>>' */
        ଅନ୍ୟଥା ଫେରନ୍ତୁ '>';
      }
      ସ୍ଥିତି '/': {
        next(ls);
        ଯଦି (check_next1(ls, '/')) ଫେରନ୍ତୁ TK_IDIV;  /* '//' */
        ଅନ୍ୟଥା ଫେରନ୍ତୁ '/';
      }
      ସ୍ଥିତି '~': {
        next(ls);
        ଯଦି (check_next1(ls, '=')) ଫେରନ୍ତୁ TK_NE;  /* '~=' */
        ଅନ୍ୟଥା ଫେରନ୍ତୁ '~';
      }
      ସ୍ଥିତି ':': {
        next(ls);
        ଯଦି (check_next1(ls, ':')) ଫେରନ୍ତୁ TK_DBCOLON;  /* '::' */
        ଅନ୍ୟଥା ଫେରନ୍ତୁ ':';
      }
      ସ୍ଥିତି '"': ସ୍ଥିତି '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        ଫେରନ୍ତୁ TK_STRING;
      }
      ସ୍ଥିତି '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        ଯଦି (check_next1(ls, '.')) {
          ଯଦି (check_next1(ls, '.'))
            ଫେରନ୍ତୁ TK_DOTS;   /* '...' */
          ଅନ୍ୟଥା ଫେରନ୍ତୁ TK_CONCAT;   /* '..' */
        }
        ଅନ୍ୟଥା ଯଦି (!lisdigit(ls->current)) ଫେରନ୍ତୁ '.';
        ଅନ୍ୟଥା ଫେରନ୍ତୁ read_numeral(ls, seminfo);
      }
      ସ୍ଥିତି '0': ସ୍ଥିତି '1': ସ୍ଥିତି '2': ସ୍ଥିତି '3': ସ୍ଥିତି '4':
      ସ୍ଥିତି '5': ସ୍ଥିତି '6': ସ୍ଥିତି '7': ସ୍ଥିତି '8': ସ୍ଥିତି '9': {
        ଫେରନ୍ତୁ read_numeral(ls, seminfo);
      }
      ସ୍ଥିତି EOZ: {
        ଫେରନ୍ତୁ TK_EOS;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ଯଦି (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          କର {
            save_and_next(ls);
          } ଯେତେବେଳେ (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          ଯଦି (isreserved(ts))   /* reserved word? */
            ଫେରନ୍ତୁ ts->extra - 1 + FIRST_RESERVED;
          ଅନ୍ୟଥା {
            seminfo->ts = anchorstr(ls, ts);
            ଫେରନ୍ତୁ TK_NAME;
          }
        }
        ଅନ୍ୟଥା {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = ls->current;
          next(ls);
          ଫେରନ୍ତୁ c;
        }
      }
    }
  }
}


ଶୂନ୍ୟ luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  ଯଦି (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  ଅନ୍ୟଥା
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  ଫେରନ୍ତୁ ls->lookahead.token;
}

