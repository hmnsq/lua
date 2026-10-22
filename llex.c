/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <locale.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lctype.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "llex.h"
#شامل "lobject.h"
#شامل "lparser.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#اگر !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
ایستا ثابت نویسه *ثابت luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


ایستا l_noret lexerror (LexState *ls, ثابت نویسه *پیام, صحیح token);


ایستا تهی save (LexState *ls, صحیح c) {
  Mbuffer *b = ls->buff;
  اگر (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    اگر (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


تهی luaX_init (lua_State *L) {
  صحیح i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  برای (i=0; i<رقم_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ثابت نویسه *luaX_token2str (LexState *ls, صحیح token) {
  اگر (token < FIRST_RESERVED) {  /* single-byte symbols? */
    اگر (lisprint(token))
      بازگشت luaO_pushfstring(ls->L, "'%c'", token);
    وگرنه  /* control character */
      بازگشت luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  وگرنه {
    ثابت نویسه *s = luaX_tokens[token - FIRST_RESERVED];
    اگر (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      بازگشت luaO_pushfstring(ls->L, "'%s'", s);
    وگرنه  /* names, strings, and numerals */
      بازگشت s;
  }
}


ایستا ثابت نویسه *txtToken (LexState *ls, صحیح token) {
  انتخاب (token) {
    حالت TK_NAME: حالت TK_STRING:
    حالت TK_FLT: حالت TK_INT:
      save(ls, '\0');
      بازگشت luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    پیشفرض:
      بازگشت luaX_token2str(ls, token);
  }
}


ایستا l_noret lexerror (LexState *ls, ثابت نویسه *پیام, صحیح token) {
  پیام = luaG_addinfo(ls->L, پیام, ls->source, ls->linenumber);
  اگر (token)
    luaO_pushfstring(ls->L, "%s near %s", پیام, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ثابت نویسه *پیام) {
  lexerror(ls, پیام, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
ایستا TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  صحیح tag = luaH_getstr(ls->h, ts, &oldts);
  اگر (!tagisempty(tag))  /* string already present? */
    بازگشت tsvalue(&oldts);  /* use stored value */
  وگرنه {  /* create a new entry */
    TValue *stv = s2v(L->بالاترین.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->بالاترین.p--;  /* remove string from stack */
    بازگشت ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ثابت نویسه *str, size_t l) {
  بازگشت anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
ایستا تهی inclinenumber (LexState *ls) {
  صحیح old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  اگر (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  اگر (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


تهی luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    صحیح firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = پوچ;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#اگر LUA_COMPAT_GLOBAL
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


ایستا صحیح check_next1 (LexState *ls, صحیح c) {
  اگر (ls->current == c) {
    next(ls);
    بازگشت 1;
  }
  وگرنه بازگشت 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
ایستا صحیح check_next2 (LexState *ls, ثابت نویسه *set) {
  lua_assert(set[2] == '\0');
  اگر (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    بازگشت 1;
  }
  وگرنه بازگشت 0;
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
ایستا صحیح read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ثابت نویسه *expo = "Ee";
  صحیح first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  اگر (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  برای (;;) {
    اگر (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    وگرنه اگر (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    وگرنه بشکن;
  }
  اگر (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  اگر (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  اگر (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    بازگشت TK_INT;
  }
  وگرنه {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    بازگشت TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
ایستا size_t skip_sep (LexState *ls) {
  size_t شمار = 0;
  صحیح s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  وقتی (ls->current == '=') {
    save_and_next(ls);
    شمار++;
  }
  بازگشت (ls->current == s) ? شمار + 2
         : (شمار == 0) ? 1
         : 0;
}


ایستا تهی read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  صحیح سطر = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  اگر (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  برای (;;) {
    انتخاب (ls->current) {
      حالت EOZ: {  /* error */
        ثابت نویسه *what = (seminfo ? "string" : "comment");
        ثابت نویسه *پیام = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, سطر);
        lexerror(ls, پیام, TK_EOS);
        بشکن;  /* to avoid warnings */
      }
      حالت ']': {
        اگر (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          برو endloop;
        }
        بشکن;
      }
      حالت '\n': حالت '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        اگر (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        بشکن;
      }
      پیشفرض: {
        اگر (seminfo) save_and_next(ls);
        وگرنه next(ls);
      }
    }
  } endloop:
  اگر (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


ایستا تهی esccheck (LexState *ls, صحیح c, ثابت نویسه *پیام) {
  اگر (!c) {
    اگر (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, پیام, TK_STRING);
  }
}


ایستا صحیح gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  بازگشت luaO_hexavalue(ls->current);
}


ایستا صحیح readhexaesc (LexState *ls) {
  صحیح r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  بازگشت r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
ایستا l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  صحیح i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  وقتی (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  بازگشت r;
}


ایستا تهی utf8esc (LexState *ls) {
  نویسه buff[UTF8BUFFSZ];
  صحیح n = luaO_utf8esc(buff, readutf8esc(ls));
  برای (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


ایستا صحیح readdecesc (LexState *ls) {
  صحیح i;
  صحیح r = 0;  /* result accumulator */
  برای (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  بازگشت r;
}


ایستا تهی read_string (LexState *ls, صحیح del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  وقتی (ls->current != del) {
    انتخاب (ls->current) {
      حالت EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        بشکن;  /* to avoid warnings */
      حالت '\n':
      حالت '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        بشکن;  /* to avoid warnings */
      حالت '\\': {  /* escape sequences */
        صحیح c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        انتخاب (ls->current) {
          حالت 'a': c = '\a'; برو read_save;
          حالت 'b': c = '\b'; برو read_save;
          حالت 'f': c = '\f'; برو read_save;
          حالت 'n': c = '\n'; برو read_save;
          حالت 'r': c = '\r'; برو read_save;
          حالت 't': c = '\t'; برو read_save;
          حالت 'v': c = '\v'; برو read_save;
          حالت 'x': c = readhexaesc(ls); برو read_save;
          حالت 'u': utf8esc(ls);  برو no_save;
          حالت '\n': حالت '\r':
            inclinenumber(ls); c = '\n'; برو only_save;
          حالت '\\': حالت '\"': حالت '\'':
            c = ls->current; برو read_save;
          حالت EOZ: برو no_save;  /* will raise an error next loop */
          حالت 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            وقتی (lisspace(ls->current)) {
              اگر (currIsNewline(ls)) inclinenumber(ls);
              وگرنه next(ls);
            }
            برو no_save;
          }
          پیشفرض: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            برو only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: بشکن;
      }
      پیشفرض:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


ایستا صحیح llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  برای (;;) {
    انتخاب (ls->current) {
      حالت '\n': حالت '\r': {  /* line breaks */
        inclinenumber(ls);
        بشکن;
      }
      حالت ' ': حالت '\f': حالت '\t': حالت '\v': {  /* spaces */
        next(ls);
        بشکن;
      }
      حالت '-': {  /* '-' or '--' (comment) */
        next(ls);
        اگر (ls->current != '-') بازگشت '-';
        /* else is a comment */
        next(ls);
        اگر (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          اگر (sep >= 2) {
            read_long_string(ls, پوچ, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            بشکن;
          }
        }
        /* else short comment */
        وقتی (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        بشکن;
      }
      حالت '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        اگر (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          بازگشت TK_STRING;
        }
        وگرنه اگر (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        بازگشت '[';
      }
      حالت '=': {
        next(ls);
        اگر (check_next1(ls, '=')) بازگشت TK_EQ;  /* '==' */
        وگرنه بازگشت '=';
      }
      حالت '<': {
        next(ls);
        اگر (check_next1(ls, '=')) بازگشت TK_LE;  /* '<=' */
        وگرنه اگر (check_next1(ls, '<')) بازگشت TK_SHL;  /* '<<' */
        وگرنه بازگشت '<';
      }
      حالت '>': {
        next(ls);
        اگر (check_next1(ls, '=')) بازگشت TK_GE;  /* '>=' */
        وگرنه اگر (check_next1(ls, '>')) بازگشت TK_SHR;  /* '>>' */
        وگرنه بازگشت '>';
      }
      حالت '/': {
        next(ls);
        اگر (check_next1(ls, '/')) بازگشت TK_IDIV;  /* '//' */
        وگرنه بازگشت '/';
      }
      حالت '~': {
        next(ls);
        اگر (check_next1(ls, '=')) بازگشت TK_NE;  /* '~=' */
        وگرنه بازگشت '~';
      }
      حالت ':': {
        next(ls);
        اگر (check_next1(ls, ':')) بازگشت TK_DBCOLON;  /* '::' */
        وگرنه بازگشت ':';
      }
      حالت '"': حالت '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        بازگشت TK_STRING;
      }
      حالت '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        اگر (check_next1(ls, '.')) {
          اگر (check_next1(ls, '.'))
            بازگشت TK_DOTS;   /* '...' */
          وگرنه بازگشت TK_CONCAT;   /* '..' */
        }
        وگرنه اگر (!lisdigit(ls->current)) بازگشت '.';
        وگرنه بازگشت read_numeral(ls, seminfo);
      }
      حالت '0': حالت '1': حالت '2': حالت '3': حالت '4':
      حالت '5': حالت '6': حالت '7': حالت '8': حالت '9': {
        بازگشت read_numeral(ls, seminfo);
      }
      حالت EOZ: {
        بازگشت TK_EOS;
      }
      پیشفرض: {
        اگر (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          انجامبده {
            save_and_next(ls);
          } وقتی (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          اگر (isreserved(ts))   /* reserved word? */
            بازگشت ts->extra - 1 + FIRST_RESERVED;
          وگرنه {
            seminfo->ts = anchorstr(ls, ts);
            بازگشت TK_NAME;
          }
        }
        وگرنه {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          صحیح c = ls->current;
          next(ls);
          بازگشت c;
        }
      }
    }
  }
}


تهی luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  اگر (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  وگرنه
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


صحیح luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  بازگشت ls->lookahead.token;
}

