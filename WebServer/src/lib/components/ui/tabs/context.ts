import type { Writable } from "svelte/store";

export const tabsContextKey = Symbol("tabs");

export type TabsContext = {
	selected: Writable<string>;
	setValue: (value: string) => void;
};
