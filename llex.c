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



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#যদি !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
স্থির ধ্রুব বর্ণ *ধ্রুব luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


স্থির l_noret lexerror (LexState *ls, ধ্রুব বর্ণ *বার্তা, পূর্ণাঙ্ক token);


স্থির শূন্য save (LexState *ls, পূর্ণাঙ্ক c) {
  Mbuffer *b = ls->buff;
  যদি (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    যদি (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


শূন্য luaX_init (lua_State *L) {
  পূর্ণাঙ্ক i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  প্রতি (i=0; i<অঙ্ক_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ধ্রুব বর্ণ *luaX_token2str (LexState *ls, পূর্ণাঙ্ক token) {
  যদি (token < FIRST_RESERVED) {  /* single-byte symbols? */
    যদি (lisprint(token))
      প্রত্যাগম luaO_pushfstring(ls->L, "'%c'", token);
    অন্যথা  /* control character */
      প্রত্যাগম luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  অন্যথা {
    ধ্রুব বর্ণ *s = luaX_tokens[token - FIRST_RESERVED];
    যদি (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      প্রত্যাগম luaO_pushfstring(ls->L, "'%s'", s);
    অন্যথা  /* names, strings, and numerals */
      প্রত্যাগম s;
  }
}


স্থির ধ্রুব বর্ণ *txtToken (LexState *ls, পূর্ণাঙ্ক token) {
  বিকল্প (token) {
    পক্ষ TK_NAME: পক্ষ TK_STRING:
    পক্ষ TK_FLT: পক্ষ TK_INT:
      save(ls, '\0');
      প্রত্যাগম luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    স্বভাব:
      প্রত্যাগম luaX_token2str(ls, token);
  }
}


স্থির l_noret lexerror (LexState *ls, ধ্রুব বর্ণ *বার্তা, পূর্ণাঙ্ক token) {
  বার্তা = luaG_addinfo(ls->L, বার্তা, ls->source, ls->linenumber);
  যদি (token)
    luaO_pushfstring(ls->L, "%s near %s", বার্তা, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ধ্রুব বর্ণ *বার্তা) {
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
  পূর্ণাঙ্ক tag = luaH_getstr(ls->h, ts, &oldts);
  যদি (!tagisempty(tag))  /* string already present? */
    প্রত্যাগম tsvalue(&oldts);  /* use stored value */
  অন্যথা {  /* create a new entry */
    TValue *stv = s2v(L->শীর্ষ.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->শীর্ষ.p--;  /* remove string from stack */
    প্রত্যাগম ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ধ্রুব বর্ণ *str, size_t l) {
  প্রত্যাগম anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
স্থির শূন্য inclinenumber (LexState *ls) {
  পূর্ণাঙ্ক old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  যদি (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  যদি (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


শূন্য luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    পূর্ণাঙ্ক firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = রিক্ত;
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


স্থির পূর্ণাঙ্ক check_next1 (LexState *ls, পূর্ণাঙ্ক c) {
  যদি (ls->current == c) {
    next(ls);
    প্রত্যাগম 1;
  }
  অন্যথা প্রত্যাগম 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
স্থির পূর্ণাঙ্ক check_next2 (LexState *ls, ধ্রুব বর্ণ *set) {
  lua_assert(set[2] == '\0');
  যদি (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    প্রত্যাগম 1;
  }
  অন্যথা প্রত্যাগম 0;
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
স্থির পূর্ণাঙ্ক read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ধ্রুব বর্ণ *expo = "Ee";
  পূর্ণাঙ্ক first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  যদি (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  প্রতি (;;) {
    যদি (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    অন্যথা যদি (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    অন্যথা ভঙ্গ;
  }
  যদি (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  যদি (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  যদি (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    প্রত্যাগম TK_INT;
  }
  অন্যথা {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    প্রত্যাগম TK_FLT;
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
  পূর্ণাঙ্ক s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  যাবত্ (ls->current == '=') {
    save_and_next(ls);
    গুনতি++;
  }
  প্রত্যাগম (ls->current == s) ? গুনতি + 2
         : (গুনতি == 0) ? 1
         : 0;
}


স্থির শূন্য read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  পূর্ণাঙ্ক সারি = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  যদি (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  প্রতি (;;) {
    বিকল্প (ls->current) {
      পক্ষ EOZ: {  /* error */
        ধ্রুব বর্ণ *what = (seminfo ? "string" : "comment");
        ধ্রুব বর্ণ *বার্তা = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, সারি);
        lexerror(ls, বার্তা, TK_EOS);
        ভঙ্গ;  /* to avoid warnings */
      }
      পক্ষ ']': {
        যদি (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          গচ্ছ endloop;
        }
        ভঙ্গ;
      }
      পক্ষ '\n': পক্ষ '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        যদি (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        ভঙ্গ;
      }
      স্বভাব: {
        যদি (seminfo) save_and_next(ls);
        অন্যথা next(ls);
      }
    }
  } endloop:
  যদি (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


স্থির শূন্য esccheck (LexState *ls, পূর্ণাঙ্ক c, ধ্রুব বর্ণ *বার্তা) {
  যদি (!c) {
    যদি (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, বার্তা, TK_STRING);
  }
}


স্থির পূর্ণাঙ্ক gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  প্রত্যাগম luaO_hexavalue(ls->current);
}


স্থির পূর্ণাঙ্ক readhexaesc (LexState *ls) {
  পূর্ণাঙ্ক r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  প্রত্যাগম r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
স্থির l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  পূর্ণাঙ্ক i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  যাবত্ (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  প্রত্যাগম r;
}


স্থির শূন্য utf8esc (LexState *ls) {
  বর্ণ buff[UTF8BUFFSZ];
  পূর্ণাঙ্ক n = luaO_utf8esc(buff, readutf8esc(ls));
  প্রতি (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


স্থির পূর্ণাঙ্ক readdecesc (LexState *ls) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক r = 0;  /* result accumulator */
  প্রতি (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  প্রত্যাগম r;
}


স্থির শূন্য read_string (LexState *ls, পূর্ণাঙ্ক del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  যাবত্ (ls->current != del) {
    বিকল্প (ls->current) {
      পক্ষ EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        ভঙ্গ;  /* to avoid warnings */
      পক্ষ '\n':
      পক্ষ '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        ভঙ্গ;  /* to avoid warnings */
      পক্ষ '\\': {  /* escape sequences */
        পূর্ণাঙ্ক c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        বিকল্প (ls->current) {
          পক্ষ 'a': c = '\a'; গচ্ছ read_save;
          পক্ষ 'b': c = '\b'; গচ্ছ read_save;
          পক্ষ 'f': c = '\f'; গচ্ছ read_save;
          পক্ষ 'n': c = '\n'; গচ্ছ read_save;
          পক্ষ 'r': c = '\r'; গচ্ছ read_save;
          পক্ষ 't': c = '\t'; গচ্ছ read_save;
          পক্ষ 'v': c = '\v'; গচ্ছ read_save;
          পক্ষ 'x': c = readhexaesc(ls); গচ্ছ read_save;
          পক্ষ 'u': utf8esc(ls);  গচ্ছ no_save;
          পক্ষ '\n': পক্ষ '\r':
            inclinenumber(ls); c = '\n'; গচ্ছ only_save;
          পক্ষ '\\': পক্ষ '\"': পক্ষ '\'':
            c = ls->current; গচ্ছ read_save;
          পক্ষ EOZ: গচ্ছ no_save;  /* will raise an error next loop */
          পক্ষ 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            যাবত্ (lisspace(ls->current)) {
              যদি (currIsNewline(ls)) inclinenumber(ls);
              অন্যথা next(ls);
            }
            গচ্ছ no_save;
          }
          স্বভাব: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            গচ্ছ only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: ভঙ্গ;
      }
      স্বভাব:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


স্থির পূর্ণাঙ্ক llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  প্রতি (;;) {
    বিকল্প (ls->current) {
      পক্ষ '\n': পক্ষ '\r': {  /* line breaks */
        inclinenumber(ls);
        ভঙ্গ;
      }
      পক্ষ ' ': পক্ষ '\f': পক্ষ '\t': পক্ষ '\v': {  /* spaces */
        next(ls);
        ভঙ্গ;
      }
      পক্ষ '-': {  /* '-' or '--' (comment) */
        next(ls);
        যদি (ls->current != '-') প্রত্যাগম '-';
        /* else is a comment */
        next(ls);
        যদি (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          যদি (sep >= 2) {
            read_long_string(ls, রিক্ত, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            ভঙ্গ;
          }
        }
        /* else short comment */
        যাবত্ (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        ভঙ্গ;
      }
      পক্ষ '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        যদি (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          প্রত্যাগম TK_STRING;
        }
        অন্যথা যদি (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        প্রত্যাগম '[';
      }
      পক্ষ '=': {
        next(ls);
        যদি (check_next1(ls, '=')) প্রত্যাগম TK_EQ;  /* '==' */
        অন্যথা প্রত্যাগম '=';
      }
      পক্ষ '<': {
        next(ls);
        যদি (check_next1(ls, '=')) প্রত্যাগম TK_LE;  /* '<=' */
        অন্যথা যদি (check_next1(ls, '<')) প্রত্যাগম TK_SHL;  /* '<<' */
        অন্যথা প্রত্যাগম '<';
      }
      পক্ষ '>': {
        next(ls);
        যদি (check_next1(ls, '=')) প্রত্যাগম TK_GE;  /* '>=' */
        অন্যথা যদি (check_next1(ls, '>')) প্রত্যাগম TK_SHR;  /* '>>' */
        অন্যথা প্রত্যাগম '>';
      }
      পক্ষ '/': {
        next(ls);
        যদি (check_next1(ls, '/')) প্রত্যাগম TK_IDIV;  /* '//' */
        অন্যথা প্রত্যাগম '/';
      }
      পক্ষ '~': {
        next(ls);
        যদি (check_next1(ls, '=')) প্রত্যাগম TK_NE;  /* '~=' */
        অন্যথা প্রত্যাগম '~';
      }
      পক্ষ ':': {
        next(ls);
        যদি (check_next1(ls, ':')) প্রত্যাগম TK_DBCOLON;  /* '::' */
        অন্যথা প্রত্যাগম ':';
      }
      পক্ষ '"': পক্ষ '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        প্রত্যাগম TK_STRING;
      }
      পক্ষ '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        যদি (check_next1(ls, '.')) {
          যদি (check_next1(ls, '.'))
            প্রত্যাগম TK_DOTS;   /* '...' */
          অন্যথা প্রত্যাগম TK_CONCAT;   /* '..' */
        }
        অন্যথা যদি (!lisdigit(ls->current)) প্রত্যাগম '.';
        অন্যথা প্রত্যাগম read_numeral(ls, seminfo);
      }
      পক্ষ '0': পক্ষ '1': পক্ষ '2': পক্ষ '3': পক্ষ '4':
      পক্ষ '5': পক্ষ '6': পক্ষ '7': পক্ষ '8': পক্ষ '9': {
        প্রত্যাগম read_numeral(ls, seminfo);
      }
      পক্ষ EOZ: {
        প্রত্যাগম TK_EOS;
      }
      স্বভাব: {
        যদি (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          কুরু {
            save_and_next(ls);
          } যাবত্ (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          যদি (isreserved(ts))   /* reserved word? */
            প্রত্যাগম ts->extra - 1 + FIRST_RESERVED;
          অন্যথা {
            seminfo->ts = anchorstr(ls, ts);
            প্রত্যাগম TK_NAME;
          }
        }
        অন্যথা {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          পূর্ণাঙ্ক c = ls->current;
          next(ls);
          প্রত্যাগম c;
        }
      }
    }
  }
}


শূন্য luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  যদি (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  অন্যথা
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


পূর্ণাঙ্ক luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  প্রত্যাগম ls->lookahead.token;
}

