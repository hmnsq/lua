/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#تعریف_کرو llex_c
#تعریف_کرو LUA_CORE

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



#تعریف_کرو اگلا(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#اگر !defined(LUA_MINBUFFER)
#تعریف_کرو LUA_MINBUFFER   32
#اگر_ختم


#تعریف_کرو currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
ساکن ثابت حرف *ثابت lua_X_علامات [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#تعریف_کرو save_and_اگلا(ls) (save(ls, ls->current), اگلا(ls))


ساکن l_noret lexerror (LexState *ls, ثابت حرف *پیغام, صحیح token);


ساکن خالی save (LexState *ls, صحیح c) {
  Mbuffer *b = ls->buff;
  اگر (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    اگر (newsize >= (MAX_سائز/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->بفر[luaZ_bufflen(b)++] = cast_char(c);
}


خالی luaX_init (lua_State *L) {
  صحیح i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  برائے (i=0; i<عدد_RESERVED; i++) {
    TString *ts = luaS_new(L, lua_X_علامات[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ثابت حرف *luaX_token2str (LexState *ls, صحیح token) {
  اگر (token < پہلا_RESERVED) {  /* single-byte symbols? */
    اگر (lisprint(token))
      واپس luaO_pushfstring(ls->L, "'%c'", token);
    ورنہ  /* control character */
      واپس luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  ورنہ {
    ثابت حرف *s = lua_X_علامات[token - پہلا_RESERVED];
    اگر (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      واپس luaO_pushfstring(ls->L, "'%s'", s);
    ورنہ  /* names, strings, and numerals */
      واپس s;
  }
}


ساکن ثابت حرف *txtToken (LexState *ls, صحیح token) {
  انتخاب (token) {
    حالت TK_NAME: حالت TK_STRING:
    حالت TK_FLT: حالت TK_INT:
      save(ls, '\0');
      واپس luaO_pushfstring(ls->L, "'%s'", lua_Z_بفر(ls->buff));
    طےشدہ:
      واپس luaX_token2str(ls, token);
  }
}


ساکن l_noret lexerror (LexState *ls, ثابت حرف *پیغام, صحیح token) {
  پیغام = luaG_addinfo(ls->L, پیغام, ls->ماخذ, ls->linenumber);
  اگر (token)
    luaO_pushfstring(ls->L, "%s near %s", پیغام, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ثابت حرف *پیغام) {
  lexerror(ls, پیغام, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
ساکن TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  صحیح لیبل = luaH_getstr(ls->h, ts, &oldts);
  اگر (!tagisempty(لیبل))  /* string already present? */
    واپس tsvalue(&oldts);  /* use stored value */
  ورنہ {  /* create a new entry */
    TValue *stv = s2v(L->چوٹی.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    lua_H_سیٹ(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->چوٹی.p--;  /* remove string from stack */
    واپس ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ثابت حرف *str, size_t l) {
  واپس anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
ساکن خالی inclinenumber (LexState *ls) {
  صحیح old = ls->current;
  lua_assert(currIsNewline(ls));
  اگلا(ls);  /* skip '\n' or '\r' */
  اگر (currIsNewline(ls) && ls->current != old)
    اگلا(ls);  /* skip '\n\r' or '\r\n' */
  اگر (++ls->linenumber >= زیادہ_سے_زیادہ_عدد)
    lexerror(ls, "chunk has too many lines", 0);
}


خالی luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *ماخذ,
                    صحیح firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = تہی;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->ماخذ = ماخذ;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#اگر LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#اگر_ختم
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


ساکن صحیح check_next1 (LexState *ls, صحیح c) {
  اگر (ls->current == c) {
    اگلا(ls);
    واپس 1;
  }
  ورنہ واپس 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
ساکن صحیح check_next2 (LexState *ls, ثابت حرف *سیٹ) {
  lua_assert(سیٹ[2] == '\0');
  اگر (ls->current == سیٹ[0] || ls->current == سیٹ[1]) {
    save_and_اگلا(ls);
    واپس 1;
  }
  ورنہ واپس 0;
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
ساکن صحیح read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ثابت حرف *expo = "Ee";
  صحیح پہلا = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_اگلا(ls);
  اگر (پہلا == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  برائے (;;) {
    اگر (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    ورنہ اگر (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_اگلا(ls);
    ورنہ توڑو;
  }
  اگر (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_اگلا(ls);  /* force an error */
  save(ls, '\0');
  اگر (luaO_str2num(lua_Z_بفر(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  اگر (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    واپس TK_INT;
  }
  ورنہ {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    واپس TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
ساکن size_t skip_sep (LexState *ls) {
  size_t گنتی = 0;
  صحیح s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_اگلا(ls);
  جب (ls->current == '=') {
    save_and_اگلا(ls);
    گنتی++;
  }
  واپس (ls->current == s) ? گنتی + 2
         : (گنتی == 0) ? 1
         : 0;
}


ساکن خالی read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  صحیح سطر = ls->linenumber;  /* initial line (for error message) */
  save_and_اگلا(ls);  /* skip 2nd '[' */
  اگر (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  برائے (;;) {
    انتخاب (ls->current) {
      حالت EOZ: {  /* error */
        ثابت حرف *what = (seminfo ? "string" : "comment");
        ثابت حرف *پیغام = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, سطر);
        lexerror(ls, پیغام, TK_EOS);
        توڑو;  /* to avoid warnings */
      }
      حالت ']': {
        اگر (skip_sep(ls) == sep) {
          save_and_اگلا(ls);  /* skip 2nd ']' */
          جاؤ endloop;
        }
        توڑو;
      }
      حالت '\n': حالت '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        اگر (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        توڑو;
      }
      طےشدہ: {
        اگر (seminfo) save_and_اگلا(ls);
        ورنہ اگلا(ls);
      }
    }
  } endloop:
  اگر (seminfo)
    seminfo->ts = luaX_newstring(ls, lua_Z_بفر(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


ساکن خالی esccheck (LexState *ls, صحیح c, ثابت حرف *پیغام) {
  اگر (!c) {
    اگر (ls->current != EOZ)
      save_and_اگلا(ls);  /* add current to buffer for error message */
    lexerror(ls, پیغام, TK_STRING);
  }
}


ساکن صحیح gethexa (LexState *ls) {
  save_and_اگلا(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  واپس luaO_hexavalue(ls->current);
}


ساکن صحیح readhexaesc (LexState *ls) {
  صحیح r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  واپس r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
ساکن l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  صحیح i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_اگلا(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  جب (cast_void(save_and_اگلا(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  اگلا(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  واپس r;
}


ساکن خالی utf8esc (LexState *ls) {
  حرف buff[UTF8BUFFSZ];
  صحیح n = luaO_utf8esc(buff, readutf8esc(ls));
  برائے (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


ساکن صحیح readdecesc (LexState *ls) {
  صحیح i;
  صحیح r = 0;  /* result accumulator */
  برائے (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_اگلا(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  واپس r;
}


ساکن خالی read_string (LexState *ls, صحیح del, SemInfo *seminfo) {
  save_and_اگلا(ls);  /* keep delimiter (for error messages) */
  جب (ls->current != del) {
    انتخاب (ls->current) {
      حالت EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        توڑو;  /* to avoid warnings */
      حالت '\n':
      حالت '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        توڑو;  /* to avoid warnings */
      حالت '\\': {  /* escape sequences */
        صحیح c;  /* final character to be saved */
        save_and_اگلا(ls);  /* keep '\\' for error messages */
        انتخاب (ls->current) {
          حالت 'a': c = '\a'; جاؤ read_save;
          حالت 'b': c = '\b'; جاؤ read_save;
          حالت 'f': c = '\f'; جاؤ read_save;
          حالت 'n': c = '\n'; جاؤ read_save;
          حالت 'r': c = '\r'; جاؤ read_save;
          حالت 't': c = '\t'; جاؤ read_save;
          حالت 'v': c = '\v'; جاؤ read_save;
          حالت 'x': c = readhexaesc(ls); جاؤ read_save;
          حالت 'u': utf8esc(ls);  جاؤ no_save;
          حالت '\n': حالت '\r':
            inclinenumber(ls); c = '\n'; جاؤ only_save;
          حالت '\\': حالت '\"': حالت '\'':
            c = ls->current; جاؤ read_save;
          حالت EOZ: جاؤ no_save;  /* will raise an error next loop */
          حالت 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            اگلا(ls);  /* skip the 'z' */
            جب (lisspace(ls->current)) {
              اگر (currIsNewline(ls)) inclinenumber(ls);
              ورنہ اگلا(ls);
            }
            جاؤ no_save;
          }
          طےشدہ: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            جاؤ only_save;
          }
        }
       read_save:
         اگلا(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: توڑو;
      }
      طےشدہ:
        save_and_اگلا(ls);
    }
  }
  save_and_اگلا(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, lua_Z_بفر(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


ساکن صحیح llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  برائے (;;) {
    انتخاب (ls->current) {
      حالت '\n': حالت '\r': {  /* line breaks */
        inclinenumber(ls);
        توڑو;
      }
      حالت ' ': حالت '\f': حالت '\t': حالت '\v': {  /* spaces */
        اگلا(ls);
        توڑو;
      }
      حالت '-': {  /* '-' or '--' (comment) */
        اگلا(ls);
        اگر (ls->current != '-') واپس '-';
        /* else is a comment */
        اگلا(ls);
        اگر (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          اگر (sep >= 2) {
            read_long_string(ls, تہی, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            توڑو;
          }
        }
        /* else short comment */
        جب (!currIsNewline(ls) && ls->current != EOZ)
          اگلا(ls);  /* skip until end of line (or end of file) */
        توڑو;
      }
      حالت '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        اگر (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          واپس TK_STRING;
        }
        ورنہ اگر (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        واپس '[';
      }
      حالت '=': {
        اگلا(ls);
        اگر (check_next1(ls, '=')) واپس TK_EQ;  /* '==' */
        ورنہ واپس '=';
      }
      حالت '<': {
        اگلا(ls);
        اگر (check_next1(ls, '=')) واپس TK_LE;  /* '<=' */
        ورنہ اگر (check_next1(ls, '<')) واپس TK_SHL;  /* '<<' */
        ورنہ واپس '<';
      }
      حالت '>': {
        اگلا(ls);
        اگر (check_next1(ls, '=')) واپس TK_GE;  /* '>=' */
        ورنہ اگر (check_next1(ls, '>')) واپس TK_SHR;  /* '>>' */
        ورنہ واپس '>';
      }
      حالت '/': {
        اگلا(ls);
        اگر (check_next1(ls, '/')) واپس TK_IDIV;  /* '//' */
        ورنہ واپس '/';
      }
      حالت '~': {
        اگلا(ls);
        اگر (check_next1(ls, '=')) واپس TK_NE;  /* '~=' */
        ورنہ واپس '~';
      }
      حالت ':': {
        اگلا(ls);
        اگر (check_next1(ls, ':')) واپس TK_DBCOLON;  /* '::' */
        ورنہ واپس ':';
      }
      حالت '"': حالت '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        واپس TK_STRING;
      }
      حالت '.': {  /* '.', '..', '...', or number */
        save_and_اگلا(ls);
        اگر (check_next1(ls, '.')) {
          اگر (check_next1(ls, '.'))
            واپس TK_DOTS;   /* '...' */
          ورنہ واپس TK_CONCAT;   /* '..' */
        }
        ورنہ اگر (!lisdigit(ls->current)) واپس '.';
        ورنہ واپس read_numeral(ls, seminfo);
      }
      حالت '0': حالت '1': حالت '2': حالت '3': حالت '4':
      حالت '5': حالت '6': حالت '7': حالت '8': حالت '9': {
        واپس read_numeral(ls, seminfo);
      }
      حالت EOZ: {
        واپس TK_EOS;
      }
      طےشدہ: {
        اگر (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          کرو {
            save_and_اگلا(ls);
          } جب (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, lua_Z_بفر(ls->buff),
                                   luaZ_bufflen(ls->buff));
          اگر (isreserved(ts))   /* reserved word? */
            واپس ts->extra - 1 + پہلا_RESERVED;
          ورنہ {
            seminfo->ts = anchorstr(ls, ts);
            واپس TK_NAME;
          }
        }
        ورنہ {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          صحیح c = ls->current;
          اگلا(ls);
          واپس c;
        }
      }
    }
  }
}


خالی lua_X_اگلا (LexState *ls) {
  ls->lastline = ls->linenumber;
  اگر (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  ورنہ
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


صحیح luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  واپس ls->lookahead.token;
}

