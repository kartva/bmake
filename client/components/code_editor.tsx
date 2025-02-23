"use client";

import React, { useContext, useEffect, useRef, useState } from "react";
import { defaultKeymap, history, historyKeymap } from "@codemirror/commands";
import { highlightSelectionMatches, searchKeymap } from "@codemirror/search";
import { crosshairCursor, drawSelection, dropCursor, EditorView, highlightActiveLine, highlightActiveLineGutter, highlightSpecialChars, keymap, lineNumbers, rectangularSelection, ViewUpdate } from "@codemirror/view";
import { EditorState, Text as CMText, ChangeSet, Extension } from "@codemirror/state";
import { lua } from "@codemirror/legacy-modes/mode/lua";
import { AppCtx, Theme } from "./wrapper";
import { Button } from "./util";
import { bracketMatching, defaultHighlightStyle, foldGutter, indentOnInput, StreamLanguage, syntaxHighlighting } from "@codemirror/language";
import { autocompletion, closeBrackets } from "@codemirror/autocomplete";

const mainTheme = (err: boolean, theme: Theme) => EditorView.theme({
	".cm-gutters": {
		"backgroundColor": theme=="dark" ? "#1e1e1e" : "#e3e4e6",
		"color": theme=="dark" ? "#838383" : "#7e7f80"
	},
	"&": {
		"backgroundColor": theme=="dark" ? "#1e1e1e" : "#e5e7eb",
		"color": err ? (theme=="dark" ? "#ef4444" : "#dc2626") : (theme=="dark" ? "#9cdcfe" : "#2563eb"),
		"flex-grow": "1",
		width: "0",
		"border-radius": "0.5rem",
		"outline": theme=="dark" ? "2px solid #52525b" : "2px solid #9ca3af",
		"padding": "2px",
		"transition-property": "outline",
		"transition-timing-function": "cubic-bezier(0.4, 0, 0.2, 1)",
		"transition-duration": "300ms",
	},
	"&.cm-editor.cm-focused": {
		"outline": theme=="dark" ? "2px solid #3B82F6" : "2px solid #2563eb",
	},
	"&.cm-editor .cm-scroller": {
		"fontFamily": "Menlo, Monaco, Consolas, \"Andale Mono\", \"Ubuntu Mono\", \"Courier New\", monospace"
	},
	".cm-content": {
		"caretColor": theme=="dark" ? "#c6c6c6" : "#4b5563"
	},
	".cm-cursor, .cm-dropCursor": {
		"borderLeftColor": theme=="dark" ? "#c6c6c6" : "#4b5563"
	},
	".cm-activeLine": {
		"backgroundColor": theme=="dark" ? "#ffffff0f" : "#99999926"
	},
	".cm-activeLineGutter": {
		"color": theme=="dark" ? "#c7c5c3" : "#000",
		"backgroundColor": theme=="dark" ? "#ffffff0f" : "#99999926"
	},
	"&.cm-focused .cm-selectionBackground, & .cm-line::selection, & .cm-selectionLayer .cm-selectionBackground, .cm-content ::selection": {
		"background": theme=="dark" ? "#6199ff2f !important" : "#add6ff !important"
	},
	"& .cm-selectionMatch": {
		"backgroundColor": theme=="dark" ? "#72a1ff59" : "#a8ac94"
	}
}, { dark: theme=="dark" });

export const baseExt = (err: boolean, theme: Theme) => [
  highlightActiveLine(),
	mainTheme(err, theme),
	rectangularSelection(),
	crosshairCursor(),
	EditorView.contentAttributes.of({tabindex: "10"}),
	keymap.of([
		...defaultKeymap,
		...searchKeymap,
		...historyKeymap
	]),
	EditorState.allowMultipleSelections.of(true),
	lineNumbers(),
  highlightActiveLineGutter(),
  highlightSpecialChars(),
  history(),
  foldGutter(),
  drawSelection(),
  dropCursor(),
  indentOnInput(),
  syntaxHighlighting(defaultHighlightStyle, {fallback: true}),
  bracketMatching(),
  closeBrackets(),
  autocompletion(),
  highlightSelectionMatches(),
	StreamLanguage.define(lua)
];

type EditorType = { type: "err" } | { type: "out" } | { type: "edit", onc: (x: ViewUpdate)=>void };
const makeBasesForTheme = (x: Theme) => ({
	err: baseExt(true, x),
	out: baseExt(false, x),
	edit: baseExt(false, x)
});

const bases: Record<Theme, Record<EditorType["type"], Extension>> = {
	light: makeBasesForTheme("light"), dark: makeBasesForTheme("dark")
};

export const exts = ({type, onc, theme}: EditorType&{onc?: (x: ViewUpdate)=>void, theme: Theme}) => [
	bases[theme][type],
	...onc!=undefined ? [
		EditorView.updateListener.of(onc),
	] : []
];

export function CMEditor({source, setSource}: {
	source: string, setSource: (x: string)=>void
}) {
	const [v,setV] = useState<{txt: CMText|null, lastSrcChange: ChangeSet|null}>({
		txt: null, lastSrcChange: null
	});
	const [editor, setEditor] = useState<EditorView|null>(null);
	const cmDiv = useRef<HTMLDivElement>(null);
	const theme = useContext(AppCtx).theme;

	useEffect(() => {
		const edit = new EditorView({
			parent: cmDiv.current!,
			extensions: exts({type: "edit", theme, onc(x) {
				if (x.docChanged) setV(s=>{
					if (s.lastSrcChange==x.changes) return s;
					else return {...s, txt: x.state.doc};
				});
			}})
		});

		setEditor(edit);
		return () => edit.destroy();
	}, [theme]);

	useEffect(()=>{
		if (editor==null) return;

		const upd = ()=>{
			const txt = CMText.of(source.split("\n"));
			if (editor.state.doc.eq(txt)) {
				setV({lastSrcChange: v.lastSrcChange, txt: null})
				return;
			}

			const l = editor.state.doc.length;
			const changes = ChangeSet.of({from: 0, to: l, insert: txt},l);
			editor.dispatch({changes});

			setV({lastSrcChange: changes, txt: null});
		};

		//no pending changes
		if (v.txt==null) return upd();

		//ample time for debounce below + file update
		//if anything fails fallback to source
		const tm = setTimeout(upd, 1200);
		return ()=>clearTimeout(tm);
	}, [source, v.txt, editor]);

	useEffect(()=>{
		const c = v.txt;
		if (c!=null) {
			const tm = setTimeout(()=>setSource(c.toString()), 500);
			return () => clearTimeout(tm);
		} else {
			return ()=>{};
		}
	}, [v.txt]);

	return (
		<div className="flex flex-col items-center gap-2">
			<div ref={cmDiv} className="flex flex-row w-full" />
		</div>
	);
}