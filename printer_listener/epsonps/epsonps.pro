% Prolog for epsonps.c, by Russell Lang 19910507
/M {moveto} def
/L {lineto} def
/S {show} def
/pica    { /fwidth xdpi 10 idiv def /fheight ydpi 6 idiv def } def
/elite   { /fwidth xdpi 12 idiv def /fheight ydpi 6 idiv def } def
/fifteen { /fwidth xdpi 15 idiv def /fheight ydpi 6 idiv def } def
/condensed  { /fwidth fwidth 3 mul 5 idiv def } def
/doublewidth { /fwidth fwidth 2 mul def } def
/doubleheight { /fwidth fwidth 2 mul def /fheight fheight 2 mul def } def
/quadheight { /fwidth fwidth 4 mul def /fheight fheight 4 mul def } def
/normalfont {/Epson findfont
  [fwidth 10 6 div mul 0 0 fheight neg 0 0] makefont setfont} def
/boldfont {/Epson-Bold findfont
  [fwidth 10 6 div mul 0 0 fheight neg 0 0] makefont setfont} def
/italicfont {/Epson-Oblique findfont
  [fwidth 10 6 div mul 0 0 fheight neg 0 0] makefont setfont} def
/bolditalicfont {/Epson-BoldOblique findfont
  [fwidth 10 6 div mul 0 0 fheight neg 0 0] makefont setfont} def
/ibmgraphfont {/IBMgraphic findfont
  [fwidth 10 6 div mul 0 0 fheight neg 0 0] makefont setfont} def
/graphicbuf 2 string def
% copy StandardEncoding and call the new version IBMEncoding
StandardEncoding dup length array copy /IBMEncoding exch def
% Re Encode a font with IBMEncoding vector
/IBMEncode {
  /BasefontDict BasefontName findfont def
  /NewfontDict BasefontDict maxlength dict def
  BasefontDict
  { exch dup /FID ne
    { dup /Encoding eq
      { exch pop IBMEncoding NewfontDict 3 1 roll put }
      { exch NewfontDict 3 1 roll put }
      ifelse
    }
    { pop pop }
    ifelse
  } forall
  NewfontDict /FontName NewfontName put
  NewfontName NewfontDict definefont pop
} def
% Load a vector into the current Encoding Vector
/EncodingLoad { aload length 2 idiv
  {Encoding 3 1 roll put} repeat} def
% Create new Epson fonts from Courier fonts, with IBMEncoding
/BasefontName /Courier def /NewfontName /Epson def
IBMEncode
/BasefontName /Courier-Bold def /NewfontName /Epson-Bold def
IBMEncode
/BasefontName /Courier-BoldOblique def /NewfontName /Epson-BoldOblique def
IBMEncode
/BasefontName /Courier-Oblique def /NewfontName /Epson-Oblique def
IBMEncode
% add the extra character encodings for IBMEncoding
[ 128 /Ccedilla 129 /udieresis 130 /eacute 131 /acircumflex
  132 /adieresis 133 /agrave 134 /aring 135 /ccedilla 136 /ecircumflex
  137 /edieresis 138 /egrave 139 /idieresis 140 /icircumflex 141 /igrave
  142 /Adieresis 143 /Aring 144 /Eacute
  145 /ae % not in Courier.afm, but seems to be present
  146 /AE % not in Courier.afm, but seems to be present
  147 /ocircumflex 148 /odieresis 149 /ograve 150 /ucircumflex 151 /ugrave
  152 /ydieresis 153 /Odieresis 154 /Udieresis 155 /cent 156 /sterling 157 /yen
  158 /.notdef % Peseta
  159 /florin 160 /aacute 161 /iacute 162 /oacute 163 /uacute 164 /ntilde
  165 /Ntilde 166 /ordfeminine 167 /ordmasculine 168 /questiondown
  169 /.notdef % logical ??
  170 /.notdef % logical not
  171 /.notdef % half
  172 /.notdef % quarter
  173 /exclamdown 174 /guillemotleft 175 /guillemotright
]
/Epson findfont begin EncodingLoad end
% International character sets
/USA [ 35 /numbersign 36 /dollar 64 /at
  91 /bracketleft 92 /backslash 93 /bracketright 94 /asciicircum 96 /quoteleft
  123 /braceleft 124 /bar 125 /braceright 126 /asciitilde ] def
/France [ 35 /numbersign 36 /dollar 64 /agrave
  91 /ring 92 /ccedilla 93 /section 94 /asciicircum 96 /quoteleft
  123 /eacute 124 /ugrave 125 /egrave 126 /dieresis ] def
/Germany [ 35 /numbersign 36 /dollar 64 /section
  91 /Adieresis 92 /Odieresis 93 /Udieresis 94 /asciicircum 96 /quoteleft
  123 /adieresis 124 /odieresis 125 /udieresis 126 /germandbls ] def
/UnitedKingdom [ 35 /sterling 36 /dollar 64 /at
  91 /bracketleft 92 /backslash 93 /bracketright 94 /asciicircum 96 /quoteleft
  123 /braceleft 124 /bar 125 /braceright 126 /asciitilde ] def
/DenmarkI [ 35 /numbersign 36 /dollar 64 /at
  91 /AE 92 /Oslash 93 /Aring 94 /asciicircum 96 /quoteleft
  123 /ae 124 /oslash 125 /aring 126 /asciitilde ] def
/Sweden [ 35 /numbersign 36 /currency 64 /Eacute
  91 /Adieresis 92 /Odieresis 93 /Aring 94 /Udieresis 96 /eacute
  123 /adieresis 124 /odieresis 125 /aring 126 /udieresis ] def
/Italy [ 35 /numbersign 36 /dollar 64 /at
  91 /ring 92 /backslash 93 /eacute 94 /asciicircum 96 /ugrave
  123 /agrave 124 /ograve 125 /egrave 126 /igrave ] def
/SpainI [ 35 /currency 36 /dollar 64 /at  % 35 should be peseta
  91 /exclamdown 92 /Ntilde 93 /questiondown 94 /asciicircum 96 /quoteleft
  123 /dieresis 124 /ntilde 125 /braceright 126 /asciitilde ] def
/Japan [ 35 /numbersign 36 /dollar 64 /at
  91 /bracketleft 92 /yen 93 /bracketright 94 /asciicircum 96 /quoteleft
  123 /braceleft 124 /bar 125 /braceright 126 /asciitilde ] def
/Norway [ 35 /numbersign 36 /currency 64 /Eacute
  91 /AE 92 /Oslash 93 /Aring 94 /Udieresis 96 /eacute
  123 /ae 124 /oslash 125 /aring 126 /udieresis ] def
/DenmarkII [ 35 /numbersign 36 /dollar 64 /Eacute
  91 /AE 92 /Oslash 93 /Aring 94 /Udieresis 96 /eacute
  123 /ae 124 /oslash 125 /aring 126 /udieresis ] def
/SpainII [ 35 /numbersign 36 /dollar 64 /aacute
  91 /exclamdown 92 /Ntilde 93 /questiondown 94 /eacute 96 /quoteleft
  123 /iacute 124 /ntilde 125 /oacute 126 /uacute ] def
/LatinAmerica [ 35 /numbersign 36 /dollar 64 /aacute
  91 /exclamdown 92 /Ntilde 93 /questiondown 94 /eacute 96 /udieresis
  123 /iacute 124 /ntilde 125 /oacute 126 /uacute ] def
% set the international character set
% this relies on all four Epson fonts using the same copy of IBMEncoding
/InternationalSet {/Epson findfont begin EncodingLoad end} def
%%BeginFont: IBMgraphic
12 dict begin % start dictionary for font
% add FontInfo dictionary
/FontInfo 2 dict dup begin
/Notice (A partial IBM PC graphic characters font by Russell Lang 19910507) readonly def
/FullName (IBMgraphic) readonly def
end readonly def
% fill in rest of IBMgraphic dictionary
/FontName /IBMgraphic def
/PaintType 3 def   % fill or stroke
/FontType 3 def    % user defined
/FontMatrix [ 0.1 0 0 0.1 0 0 ] readonly def
/FontBBox [ 0 -2 6 8 ] readonly def
/Encoding 256 array def   % allocate the Encoding array
  0 1 255 { Encoding exch /.notdef put} for
% look up an IBM or Epson manual for explanation of these codes
[
  000 /blank 001 /face 002 /facereverse 003 /heart 004 /diamond 005 /club
  006 /spade 007 /disc 008 /discreverse 009 /circle 010 /circlereverse
  011 /male 012 /female 013 /note 014 /notes 015 /lamp
  016 /triangleright 017 /triangleleft 018 /arrowupdown 019 /exclamdouble
  020 /paragraph 021 /section 022 /I022 023 /arrowupdownunder 024 /arrowup
  025 /arrowdown 026 /arrowright 027 /arrowleft 028 /I028 029 /arrowboth
  030 /triangleup 031 /triangledown
  127 /delete 158 /peseta 169 /I169
  170 /logicalnot 171 /half 172 /quarter
  176 /lightgrey 177 /mediumgrey 178 /darkgrey
  179 /I179 180 /I180 181 /I181 182 /I182 183 /I183 184 /I184 185 /I185
  186 /I186 187 /I187 188 /I188 189 /I189 190 /I190 191 /I191 192 /I192
  193 /I193 194 /I194 195 /I195 196 /I196 197 /I197 198 /I198 199 /I199
  200 /I200 201 /I201 202 /I202 203 /I203 204 /I204 205 /I205 206 /I206
  207 /I207 208 /I208 209 /I209 210 /I210 211 /I211 212 /I212 213 /I213
  214 /I214 215 /I215 216 /I216 217 /I217 218 /I218
  219 /blockfull 220 /blockbot 221 /blockleft 222 /blockright 223 /blocktop
  224 /alpha 225 /beta 226 /Gamma 227 /pi 228 /Sigma 229 /sigma 230 /mu
  231 /tau 232 /Phi 233 /Theta 234 /Omega 235 /delta 236 /infinity
  237 /emptyset 238 /element 239 /intersection 240 /equivalence 241 /plusminus
  242 /greaterequal 243 /lessequal 244 /integraltop 245 /integralbot
  246 /divide 247 /approxequal 248 /degree 249 /bullet 250 /dotmath
  251 /radical 252 /supern 253 /super2 254 /slug
]
EncodingLoad
% show string from the Symbol font
/SymbolShow { % char_code matrix --
  /Symbol findfont exch makefont setfont
  0 0 moveto show
} def
% show string from the Courier font
/CourierShow { % char_code matrix --
  /Courier findfont exch makefont setfont
  0 0 moveto show
} def
% Build Character into cache
/BuildChar {  %  font_dictionary char_code --
  exch begin                      % this font dictionary
  Encoding exch get               % get character name
  gsave                           % save graphics state
  0.5 setlinewidth		  % width of stroked lines
  newpath
  6 0                             % width vector (mono spaced)
  0 -2 6 8 setcachedevice         % cache this bounding box
  CharProcs exch get              % get procedure
  exec                            % draw it
  grestore		          % restore graphics state
  end                             % end of font dictionary
} def
% add CharProcs dictionary which describes each character
/CharProcs 118 dict dup begin
/.notdef {} def
/blank {} def
/face {5.75 4.5 M 4 4.5 1.75 0 90 arc 2 4.5 1.75 90 180 arc
  2 1.5 1.75 180 270 arc 4 1.5 1.75 270 360 arc 5.75 4.5 lineto
  5.25 4.5 M  4 1.5 1.25 360 270 arcn 2 1.5 1.25 270 180 arcn
  2 4.5 1.25 180 90 arcn 4 4.5 1.25 90 0 arcn 5.25 4.5 lineto
  2.5 4 M 2 4 0.5 360 0 arcn
  4.5 4 M 4 4 0.5 360 0 arcn
  1.375 2.185 M 3 5 3.25 240 300 arc 3 5 3.75 300 240 arcn closepath
  fill} def
/facereverse {5.75 4.5 M 4 4.5 1.75 0 90 arc 2 4.5 1.75 90 180 arc
  2 1.5 1.75 180 270 arc 4 1.5 1.75 270 360 arc 5.75 4.5 lineto
  2.5 4 M 2 4 0.5 360 0 arcn
  4.5 4 M 4 4 0.5 360 0 arcn
  1.375 2.185 M 3 5 3.25 240 300 arc 3 5 3.75 300 240 arcn closepath
  fill} def
/heart {(\251) [8 0 0 10 0 0] SymbolShow} def
/diamond {(\250) [8 0 0 10 0 0] SymbolShow} def
/club {(\247) [8 0 0 10 0 0] SymbolShow} def
/spade {(\252) [8 0 0 10 0 0] SymbolShow} def
/disc {3 3 1.5 0 360 arc closepath fill} def
/discreverse {0 -2 M 6 -2 L 6 8 L 0 8 L closepath
  4.5 3 M 3 3 1.5 360 0 arcn closepath fill} def
/circle {3 3 2.5 0 360 arc 3 3 2 360 0 arcn fill} def
/circlereverse {0 -2 M 6 -2 L 6 8 L 0 8 L closepath
  5.5 3 M 3 3 2.5 360 0 arcn 5 3 M 3 3 2 0 360 arc fill} def
/male {2.5 1.5 1.5 0 360 arc
  2.5 3 M 5 5.5 L 3 5.5 M 5 5.5 L 5 3.5 L stroke } def
/female {3 4.5 1.5 0 360 arc 3 3 M 3 0 L 1.5 1.5 M 4.5 1.5 L stroke} def
/note {1.5 1.5 1 0 360 arc fill
  2.25 1.5 M 2.25 6 L 5 6 L 5 5 L 2.25 5 L stroke} def
/notes {1.5 1.5 1 0 360 arc fill 4.25 1.5 1 0 360 arc fill
  2.25 1.5 M 2.25 6 L 5 6 L 5 5 L 2.25 5 L 5 6 M 5 1.5 L stroke} def
/lamp {3 3 1 0 360 arc 3 6 M 3 4 L 3 0 M 3 2 L
  4 4 M 5 5 L 2 4 M 1 5 L 2 2 M 1 1 L 4 2 M 5 1 L stroke} def
/triangleleft  {5 0 M 1 3 L 5 6 L 5 0 L closepath fill} def
/triangleright {1 0 M 1 6 L 5 3 L 1 0 L closepath fill} def
/arrowupdown {3 0.5 M 3 5.5 L stroke
  1.5 4.5 M 3 6 L 4.5 4.5 L closepath fill
  1.5 1.5 M 3 0 L 4.5 1.5 L closepath fill} def
/exclamdouble {(!) [10 0 0 10 -1.5 0] CourierShow
  (!) [10 0 0 10  1.5 0] CourierShow} def
/paragraph {(\266) [10 0 0 10 0 0] CourierShow} def
/section {(\247) [10 0 0 10 0 0] CourierShow} def
/I022 {0 0 M 0 1.5 L 6 1.5 L 6 0 L 0 0 L closepath fill} def % cursor ??
/arrowupdownunder {CharProcs /arrowupdown get exec
  1.5 -0.25 M 4.5 -0.25 L stroke} def
/arrowup    {1.5 4.5 M 3 6 L 4.5 4.5 L closepath fill
  3 0 M 3 4.5 L stroke} def
/arrowdown  {1.5 1.5 M 3 0 L 4.5 1.5 L closepath fill
  3 1.5 M 3 6 L stroke} def
/arrowleft  {2 4.5 M 0.5 3 L 2 1.5 L closepath fill
  2 3 M 5.25 3 L stroke} def
/arrowright {4 4.5 M 5.5 3 L 4 1.5 L closepath fill
  0.25 3 M 4.5 3 L stroke} def
/I028 {0.4 setlinewidth 1 6 M 1 4 L 5 4 L stroke} def % logical ??
/arrowboth {2 3 M 4.5 3 L stroke
  2 4.5 M 0.5 3 L 2 1.5 L closepath fill
  4 4.5 M 5.5 3 L 4 1.5 L closepath fill} def
/triangleup   {1 1 M 3 5 L 5 1 L 1 1 L closepath fill} def
/triangledown {1 5 M 3 1 L 5 5 L 1 5 L closepath fill} def
% Strays
/delete {1 1 M 5 1 L 5 3 L 3 5 L 1 3 L 1 1 L closepath stroke} def
/peseta { /Courier findfont [6 0 0 10 0 0] makefont setfont
  0 0 M (P) show 2.5 0 M (t) show} def
/I169 {0.4 setlinewidth 1 2 M 1 4 L 5 4 L stroke} def % logical ??
/logicalnot {0.4 setlinewidth 1 4 M 5 4 L 5 2 L stroke} def
/half { /Courier findfont [5 0 0 5 0 0] makefont setfont 0 4 M (1) show
  2.5 0 M (2) show 0.2 setlinewidth 0.5 2 M 5.5 5 L stroke} def
/quarter { /Courier findfont [5 0 0 5 0 0] makefont setfont 0 4 M (1) show
  2.5 0 M (4) show 0.2 setlinewidth 0.5 2 M 5.5 5 L stroke} def
% Grey Blobs
/lightgrey {1 setlinewidth
  -0.5 2 8 {0 3 6 {1 index moveto currentlinewidth 0 rlineto} for pop} for
  -1.5 2 8 {1.5 3 6 {1 index moveto currentlinewidth 0 rlineto} for pop} for
  stroke} def
/mediumgrey {1 setlinewidth
  -0.5 2 8 {0 2 6 {1 index moveto currentlinewidth 0 rlineto} for pop} for
  -1.5 2 8 {1 2 6 {1 index moveto currentlinewidth 0 rlineto} for pop} for
  stroke} def
/darkgrey {1 setlinewidth
  -0.5 2 8 {0 exch M 2   0 rlineto 1 0 rmoveto 2 0 rlineto} for
  -1.5 2 8 {0 exch M 0.5 0 rlineto 1 0 rmoveto 2 0 rlineto
   1 0 rmoveto 1.5 0 rlineto} for stroke} def
% Box line line segments
/I179 {3 -2 M 3 8 L stroke} def
/I180 {3 -2 M 3 8 L 0 4 M 3 4 L stroke} def
/I181 {3 -2 M 3 8 L 0 4 M 3 4 L 0 2 M 3 2 L stroke} def
/I182 {2 -2 M 2 8 L 4 -2 M 4 8 L 0 4 M 2 4 L stroke} def
/I183 {0 4 M 4 4 L 4 -2 L 2 -2 M 2 4 L stroke} def
/I184 {0 4 M 3 4 L 3 -2 L 0 2 M 3 2 L stroke} def
/I185 {4 -2 M 4 8 L 0 4 M 2 4 L 2 8 L 0 2 M 2 2 L 2 -2 L stroke} def
/I186 {2 -2 M 2 8 L 4 -2 M 4 8 L stroke} def
/I187 {0 4 M 4 4 L 4 -2 L 0 2 M 2 2 L 2 -2 L stroke} def
/I188 {0 2 M 4 2 L 4 8 L 0 4 M 2 4 L 2 8 L stroke} def
/I189 {0 4 M 4 4 L 4 8 L 2 4 M 2 8 L stroke} def
/I190 {0 2 M 3 2 L 3 8 L 0 4 M 3 4 L stroke} def
/I191 {0 4 M 3 4 L 3 -2 L stroke} def
/I192 {3 8 M 3 4 L 6 4 L stroke} def
/I193 {0 4 M 6 4 L 3 4 M 3 8 L stroke} def
/I194 {0 4 M 6 4 L 3 4 M 3 -2 L stroke} def
/I195 {3 -2 M 3 8 L 3 4 M 6 4 L stroke} def
/I196 {0 4 M 6 4 L stroke} def
/I197 {0 4 M 6 4 L 3 -2 M 3 8 L stroke} def
/I198 {3 -2 M 3 8 L 3 4 M 6 4 L 3 2 M 6 2 L stroke} def
/I199 {2 -2 M 2 8 L 4 -2 M 4 8 L 4 4 M 6 4 L stroke} def
/I200 {2 8 M 2 2 L 6 2 L 4 8 M 4 4 L 6 4 L stroke} def
/I201 {2 -2 M 2 4 L 6 4 L 4 -2 M 4 2 L 6 2 L stroke} def
/I202 {0 2 M 6 2 L 0 4 M 2 4 L 2 8 L 4 8 M 4 4 L 6 4 L stroke} def
/I203 {0 4 M 6 4 L 0 2 M 2 2 L 2 -2 L 4 -2 M 4 2 L 6 2 L stroke} def
/I204 {2 -2 M 2 8 L 4 8 M 4 4 L 6 4 L 4 -2 M 4 2 L 6 2 L stroke} def
/I205 {0 4 M 6 4 L 0 2 M 6 2 L stroke} def
/I206 {0 4 M 2 4 L 2 8 L 4 8 M 4 4 L 6 4 L 6 2 M 4 2 L 4 -2 L
  2 -2 M 2 2 L 0 2 L stroke} def
/I207 {0 2 M 6 2 L 0 4 M 6 4 L 3 4 M 3 8 L stroke} def
/I208 {0 4 M 6 4 L 2 4 M 2 8 L 4 4 M 4 8 L stroke} def
/I209 {0 4 M 6 4 L 0 2 M 6 2 L 3 2 M 3 -2 L stroke} def
/I210 {0 4 M 6 4 L 2 4 M 2 -2 L 4 4 M 4 -2 L stroke} def
/I211 {2 8 M 2 4 L 6 4 L 4 8 M 4 4 L stroke} def
/I212 {3 8 M 3 2 L 6 2 L 3 4 M 6 4 L stroke} def
/I213 {3 -2 M 3 4 L 6 4 L 3 2 M 6 2 L stroke} def
/I214 {2 -2 M 2 4 L 6 4 L 4 -2 M 4 4 L stroke} def
/I215 {2 -2 M 2 8 L 4 -2 M 4 8 L 0 4 M 6 4 L stroke} def
/I216 {3 -2 M 3 8 L 0 4 M 6 4 L 0 2 M 6 2 L stroke} def
/I217 {0 4 M 3 4 L 3 8 L stroke} def
/I218 {3 -2 M 3 4 L 6 4 L stroke} def
% Block segments
/blockfull  {0 -2 M 6 -2 L 6 8 L 0 8 L 0 -2 L closepath fill} def
/blockbot   {0 -2 M 6 -2 L 6 3 L 0 3 L 0 -2 L closepath fill} def
/blockleft  {0 -2 M 3 -2 L 3 8 L 0 8 L 0 -2 L closepath fill} def
/blockright {3 -2 M 6 -2 L 6 8 L 3 8 L 3 -2 L closepath fill} def
/blocktop   {0 3 M 6 3 L 6 8 L 0 8 L 0 3 L closepath fill} def
% from symbol font
/alpha {(a) [9.5 0 0 10 0 0] SymbolShow} def
/beta {(b) [10 0 0 10 0 0] SymbolShow} def
/Gamma {(G) [10 0 0 10 0 0] SymbolShow} def
/pi {(p) [10 0 0 10 0 0] SymbolShow} def
/Sigma {(S) [10 0 0 10 0 0] SymbolShow} def
/sigma {(s) [10 0 0 10 0 0] SymbolShow} def
/mu {(m) [10 0 0 10 0 0] SymbolShow} def
/tau {(t) [10 0 0 10 0.5 0] SymbolShow} def
/Phi {(F) [8 0 0 10 0 0] SymbolShow} def
/Theta {(Q) [8 0 0 10 0 0] SymbolShow} def
/Omega {(W) [8 0 0 10 0 0] SymbolShow} def
/delta {(d) [10 0 0 10 0 0] SymbolShow} def
/infinity {(\245) [8.5 0 0 10 0 0] SymbolShow} def
/emptyset {(\306) [7.5 0 0 10 0 0] SymbolShow} def
/element {(\316) [8.5 0 0 10 0 0] SymbolShow} def
/intersection {(\307) [8 0 0 10 0 0] SymbolShow} def
/equivalence {(\272) [10 0 0 10 0 0] SymbolShow} def
/plusminus {(\261) [10 0 0 10 0 0] SymbolShow} def
/greaterequal {(\263) [10 0 0 10 0 0] SymbolShow} def
/lessequal {(\243) [10 0 0 10 0 0] SymbolShow} def
% integral pieces
/integraltop {3 -2 M 3 6 L 4 6 1 180 0 arcn stroke} def
/integralbot {3 8 M 3 0 L 2 0 1 0 180 arcn stroke} def
% from symbol font
/divide {(\270) [10 0 0 10 0 0] SymbolShow} def
/approxequal {(\273) [10 0 0 10 0 0] SymbolShow} def
/degree {(\260) [10 0 0 10 0 0] SymbolShow} def
/bullet {(\267) [10 0 0 10 0 0] SymbolShow} def
/dotmath {(\327) [10 0 0 10 0 0] SymbolShow} def
/radical {(\326) [10 0 0 10 0 0] SymbolShow} def
% from courier font
/supern { /Courier findfont [7 0 0 5 0 0] makefont setfont
  0 4 moveto (n) show } def
/super2 { /Courier findfont [7 0 0 5 0 0] makefont setfont
  0 4 moveto (2) show } def
% slug
/slug {1.5 0 M 4.5 0 L 4.5 5 L 1.5 5 L 1.5 0 L closepath fill} def
end readonly def % end CharProcs
currentdict end  % end the font dictionary, and leave it on the stack
dup /FontName get exch definefont pop % define the new font
%%EndFont
% end of epsonps.pro
