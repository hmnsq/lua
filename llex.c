/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#నిర్వచించు llex_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <locale.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lctype.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "llex.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lparser.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "lzio.h"



#నిర్వచించు తదుపరి(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#ఒకవేళ !defined(LUA_MINBUFFER)
#నిర్వచించు LUA_MINBUFFER   32
#అయితే_ముగింపు


#నిర్వచించు currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి lua_X_టోకెన్లు [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#నిర్వచించు save_and_తదుపరి(ls) (save(ls, ls->current), తదుపరి(ls))


స్థిర l_noret lexerror (LexState *ls, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం token);


స్థిర శూన్య save (LexState *ls, పూర్ణాంకం c) {
  Mbuffer *b = ls->buff;
  ఒకవేళ (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    ఒకవేళ (newsize >= (MAX_కొలత/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->బఫరు[luaZ_bufflen(b)++] = cast_char(c);
}


శూన్య luaX_init (lua_State *L) {
  పూర్ణాంకం i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  కొరకు (i=0; i<అంకె_RESERVED; i++) {
    TString *ts = luaS_new(L, lua_X_టోకెన్లు[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


స్థిరరాశి అక్షరం *luaX_token2str (LexState *ls, పూర్ణాంకం token) {
  ఒకవేళ (token < మొదటి_RESERVED) {  /* single-byte symbols? */
    ఒకవేళ (lisprint(token))
      తిరిగి luaO_pushfstring(ls->L, "'%c'", token);
    లేకపోతే  /* control character */
      తిరిగి luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  లేకపోతే {
    స్థిరరాశి అక్షరం *s = lua_X_టోకెన్లు[token - మొదటి_RESERVED];
    ఒకవేళ (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      తిరిగి luaO_pushfstring(ls->L, "'%s'", s);
    లేకపోతే  /* names, strings, and numerals */
      తిరిగి s;
  }
}


స్థిర స్థిరరాశి అక్షరం *txtToken (LexState *ls, పూర్ణాంకం token) {
  ఎంపిక (token) {
    స్థితి TK_NAME: స్థితి TK_STRING:
    స్థితి TK_FLT: స్థితి TK_INT:
      save(ls, '\0');
      తిరిగి luaO_pushfstring(ls->L, "'%s'", lua_Z_బఫరు(ls->buff));
    అప్రమేయ:
      తిరిగి luaX_token2str(ls, token);
  }
}


స్థిర l_noret lexerror (LexState *ls, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం token) {
  సందేశం = luaG_addinfo(ls->L, సందేశం, ls->ఆధారం, ls->linenumber);
  ఒకవేళ (token)
    luaO_pushfstring(ls->L, "%s near %s", సందేశం, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, స్థిరరాశి అక్షరం *సందేశం) {
  lexerror(ls, సందేశం, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
స్థిర TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  పూర్ణాంకం ట్యాగ్ = luaH_getstr(ls->h, ts, &oldts);
  ఒకవేళ (!tagisempty(ట్యాగ్))  /* string already present? */
    తిరిగి tsvalue(&oldts);  /* use stored value */
  లేకపోతే {  /* create a new entry */
    TValue *stv = s2v(L->శిఖరం.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    lua_H_సమితి(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->శిఖరం.p--;  /* remove string from stack */
    తిరిగి ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, స్థిరరాశి అక్షరం *str, size_t l) {
  తిరిగి anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
స్థిర శూన్య inclinenumber (LexState *ls) {
  పూర్ణాంకం old = ls->current;
  lua_assert(currIsNewline(ls));
  తదుపరి(ls);  /* skip '\n' or '\r' */
  ఒకవేళ (currIsNewline(ls) && ls->current != old)
    తదుపరి(ls);  /* skip '\n\r' or '\r\n' */
  ఒకవేళ (++ls->linenumber >= గరిష్ఠ_పూర్ణాంకం)
    lexerror(ls, "chunk has too many lines", 0);
}


శూన్య luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *ఆధారం,
                    పూర్ణాంకం firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = ఖాళీ;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->ఆధారం = ఆధారం;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#ఒకవేళ LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#అయితే_ముగింపు
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


స్థిర పూర్ణాంకం check_next1 (LexState *ls, పూర్ణాంకం c) {
  ఒకవేళ (ls->current == c) {
    తదుపరి(ls);
    తిరిగి 1;
  }
  లేకపోతే తిరిగి 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
స్థిర పూర్ణాంకం check_next2 (LexState *ls, స్థిరరాశి అక్షరం *సమితి) {
  lua_assert(సమితి[2] == '\0');
  ఒకవేళ (ls->current == సమితి[0] || ls->current == సమితి[1]) {
    save_and_తదుపరి(ls);
    తిరిగి 1;
  }
  లేకపోతే తిరిగి 0;
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
స్థిర పూర్ణాంకం read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  స్థిరరాశి అక్షరం *expo = "Ee";
  పూర్ణాంకం మొదటి = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_తదుపరి(ls);
  ఒకవేళ (మొదటి == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  కొరకు (;;) {
    ఒకవేళ (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    లేకపోతే ఒకవేళ (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_తదుపరి(ls);
    లేకపోతే ఆపు;
  }
  ఒకవేళ (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_తదుపరి(ls);  /* force an error */
  save(ls, '\0');
  ఒకవేళ (luaO_str2num(lua_Z_బఫరు(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  ఒకవేళ (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    తిరిగి TK_INT;
  }
  లేకపోతే {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    తిరిగి TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
స్థిర size_t skip_sep (LexState *ls) {
  size_t లెక్క = 0;
  పూర్ణాంకం s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_తదుపరి(ls);
  అయితే (ls->current == '=') {
    save_and_తదుపరి(ls);
    లెక్క++;
  }
  తిరిగి (ls->current == s) ? లెక్క + 2
         : (లెక్క == 0) ? 1
         : 0;
}


స్థిర శూన్య read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  పూర్ణాంకం పంక్తి = ls->linenumber;  /* initial line (for error message) */
  save_and_తదుపరి(ls);  /* skip 2nd '[' */
  ఒకవేళ (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  కొరకు (;;) {
    ఎంపిక (ls->current) {
      స్థితి EOZ: {  /* error */
        స్థిరరాశి అక్షరం *what = (seminfo ? "string" : "comment");
        స్థిరరాశి అక్షరం *సందేశం = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, పంక్తి);
        lexerror(ls, సందేశం, TK_EOS);
        ఆపు;  /* to avoid warnings */
      }
      స్థితి ']': {
        ఒకవేళ (skip_sep(ls) == sep) {
          save_and_తదుపరి(ls);  /* skip 2nd ']' */
          వెళ్ళు endloop;
        }
        ఆపు;
      }
      స్థితి '\n': స్థితి '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        ఒకవేళ (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        ఆపు;
      }
      అప్రమేయ: {
        ఒకవేళ (seminfo) save_and_తదుపరి(ls);
        లేకపోతే తదుపరి(ls);
      }
    }
  } endloop:
  ఒకవేళ (seminfo)
    seminfo->ts = luaX_newstring(ls, lua_Z_బఫరు(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


స్థిర శూన్య esccheck (LexState *ls, పూర్ణాంకం c, స్థిరరాశి అక్షరం *సందేశం) {
  ఒకవేళ (!c) {
    ఒకవేళ (ls->current != EOZ)
      save_and_తదుపరి(ls);  /* add current to buffer for error message */
    lexerror(ls, సందేశం, TK_STRING);
  }
}


స్థిర పూర్ణాంకం gethexa (LexState *ls) {
  save_and_తదుపరి(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  తిరిగి luaO_hexavalue(ls->current);
}


స్థిర పూర్ణాంకం readhexaesc (LexState *ls) {
  పూర్ణాంకం r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  తిరిగి r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
స్థిర l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  పూర్ణాంకం i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_తదుపరి(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  అయితే (cast_void(save_and_తదుపరి(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  తదుపరి(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  తిరిగి r;
}


స్థిర శూన్య utf8esc (LexState *ls) {
  అక్షరం buff[UTF8BUFFSZ];
  పూర్ణాంకం n = luaO_utf8esc(buff, readutf8esc(ls));
  కొరకు (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


స్థిర పూర్ణాంకం readdecesc (LexState *ls) {
  పూర్ణాంకం i;
  పూర్ణాంకం r = 0;  /* result accumulator */
  కొరకు (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_తదుపరి(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  తిరిగి r;
}


స్థిర శూన్య read_string (LexState *ls, పూర్ణాంకం del, SemInfo *seminfo) {
  save_and_తదుపరి(ls);  /* keep delimiter (for error messages) */
  అయితే (ls->current != del) {
    ఎంపిక (ls->current) {
      స్థితి EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        ఆపు;  /* to avoid warnings */
      స్థితి '\n':
      స్థితి '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        ఆపు;  /* to avoid warnings */
      స్థితి '\\': {  /* escape sequences */
        పూర్ణాంకం c;  /* final character to be saved */
        save_and_తదుపరి(ls);  /* keep '\\' for error messages */
        ఎంపిక (ls->current) {
          స్థితి 'a': c = '\a'; వెళ్ళు read_save;
          స్థితి 'b': c = '\b'; వెళ్ళు read_save;
          స్థితి 'f': c = '\f'; వెళ్ళు read_save;
          స్థితి 'n': c = '\n'; వెళ్ళు read_save;
          స్థితి 'r': c = '\r'; వెళ్ళు read_save;
          స్థితి 't': c = '\t'; వెళ్ళు read_save;
          స్థితి 'v': c = '\v'; వెళ్ళు read_save;
          స్థితి 'x': c = readhexaesc(ls); వెళ్ళు read_save;
          స్థితి 'u': utf8esc(ls);  వెళ్ళు no_save;
          స్థితి '\n': స్థితి '\r':
            inclinenumber(ls); c = '\n'; వెళ్ళు only_save;
          స్థితి '\\': స్థితి '\"': స్థితి '\'':
            c = ls->current; వెళ్ళు read_save;
          స్థితి EOZ: వెళ్ళు no_save;  /* will raise an error next loop */
          స్థితి 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            తదుపరి(ls);  /* skip the 'z' */
            అయితే (lisspace(ls->current)) {
              ఒకవేళ (currIsNewline(ls)) inclinenumber(ls);
              లేకపోతే తదుపరి(ls);
            }
            వెళ్ళు no_save;
          }
          అప్రమేయ: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            వెళ్ళు only_save;
          }
        }
       read_save:
         తదుపరి(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: ఆపు;
      }
      అప్రమేయ:
        save_and_తదుపరి(ls);
    }
  }
  save_and_తదుపరి(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, lua_Z_బఫరు(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


స్థిర పూర్ణాంకం llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  కొరకు (;;) {
    ఎంపిక (ls->current) {
      స్థితి '\n': స్థితి '\r': {  /* line breaks */
        inclinenumber(ls);
        ఆపు;
      }
      స్థితి ' ': స్థితి '\f': స్థితి '\t': స్థితి '\v': {  /* spaces */
        తదుపరి(ls);
        ఆపు;
      }
      స్థితి '-': {  /* '-' or '--' (comment) */
        తదుపరి(ls);
        ఒకవేళ (ls->current != '-') తిరిగి '-';
        /* else is a comment */
        తదుపరి(ls);
        ఒకవేళ (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          ఒకవేళ (sep >= 2) {
            read_long_string(ls, ఖాళీ, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            ఆపు;
          }
        }
        /* else short comment */
        అయితే (!currIsNewline(ls) && ls->current != EOZ)
          తదుపరి(ls);  /* skip until end of line (or end of file) */
        ఆపు;
      }
      స్థితి '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        ఒకవేళ (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          తిరిగి TK_STRING;
        }
        లేకపోతే ఒకవేళ (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        తిరిగి '[';
      }
      స్థితి '=': {
        తదుపరి(ls);
        ఒకవేళ (check_next1(ls, '=')) తిరిగి TK_EQ;  /* '==' */
        లేకపోతే తిరిగి '=';
      }
      స్థితి '<': {
        తదుపరి(ls);
        ఒకవేళ (check_next1(ls, '=')) తిరిగి TK_LE;  /* '<=' */
        లేకపోతే ఒకవేళ (check_next1(ls, '<')) తిరిగి TK_SHL;  /* '<<' */
        లేకపోతే తిరిగి '<';
      }
      స్థితి '>': {
        తదుపరి(ls);
        ఒకవేళ (check_next1(ls, '=')) తిరిగి TK_GE;  /* '>=' */
        లేకపోతే ఒకవేళ (check_next1(ls, '>')) తిరిగి TK_SHR;  /* '>>' */
        లేకపోతే తిరిగి '>';
      }
      స్థితి '/': {
        తదుపరి(ls);
        ఒకవేళ (check_next1(ls, '/')) తిరిగి TK_IDIV;  /* '//' */
        లేకపోతే తిరిగి '/';
      }
      స్థితి '~': {
        తదుపరి(ls);
        ఒకవేళ (check_next1(ls, '=')) తిరిగి TK_NE;  /* '~=' */
        లేకపోతే తిరిగి '~';
      }
      స్థితి ':': {
        తదుపరి(ls);
        ఒకవేళ (check_next1(ls, ':')) తిరిగి TK_DBCOLON;  /* '::' */
        లేకపోతే తిరిగి ':';
      }
      స్థితి '"': స్థితి '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        తిరిగి TK_STRING;
      }
      స్థితి '.': {  /* '.', '..', '...', or number */
        save_and_తదుపరి(ls);
        ఒకవేళ (check_next1(ls, '.')) {
          ఒకవేళ (check_next1(ls, '.'))
            తిరిగి TK_DOTS;   /* '...' */
          లేకపోతే తిరిగి TK_CONCAT;   /* '..' */
        }
        లేకపోతే ఒకవేళ (!lisdigit(ls->current)) తిరిగి '.';
        లేకపోతే తిరిగి read_numeral(ls, seminfo);
      }
      స్థితి '0': స్థితి '1': స్థితి '2': స్థితి '3': స్థితి '4':
      స్థితి '5': స్థితి '6': స్థితి '7': స్థితి '8': స్థితి '9': {
        తిరిగి read_numeral(ls, seminfo);
      }
      స్థితి EOZ: {
        తిరిగి TK_EOS;
      }
      అప్రమేయ: {
        ఒకవేళ (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          చేయి {
            save_and_తదుపరి(ls);
          } అయితే (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, lua_Z_బఫరు(ls->buff),
                                   luaZ_bufflen(ls->buff));
          ఒకవేళ (isreserved(ts))   /* reserved word? */
            తిరిగి ts->extra - 1 + మొదటి_RESERVED;
          లేకపోతే {
            seminfo->ts = anchorstr(ls, ts);
            తిరిగి TK_NAME;
          }
        }
        లేకపోతే {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          పూర్ణాంకం c = ls->current;
          తదుపరి(ls);
          తిరిగి c;
        }
      }
    }
  }
}


శూన్య lua_X_తదుపరి (LexState *ls) {
  ls->lastline = ls->linenumber;
  ఒకవేళ (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  లేకపోతే
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


పూర్ణాంకం luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  తిరిగి ls->lookahead.token;
}

