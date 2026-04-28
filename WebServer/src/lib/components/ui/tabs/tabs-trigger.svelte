<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import { getContext } from "svelte";
	import type { HTMLButtonAttributes } from "svelte/elements";
	import { tabsContextKey, type TabsContext } from "./context.js";

	type Props = WithElementRef<HTMLButtonAttributes, HTMLButtonElement> & {
		value: string;
	};

	let {
		class: className,
		ref = $bindable(null),
		value,
		type = "button",
		disabled = false,
		children,
		...restProps
	}: Props = $props();

	const { selected, setValue } = getContext<TabsContext>(tabsContextKey);
	const isSelected = $derived($selected === value);
</script>

<button
	bind:this={ref}
	data-slot="tabs-trigger"
	data-state={isSelected ? "active" : "inactive"}
	class={cn(
		"inline-flex min-h-11 items-center justify-center rounded-[1.15rem] px-4 py-2 text-sm font-medium transition-all outline-none",
		"focus-visible:border-ring focus-visible:ring-ring/50 focus-visible:ring-[3px]",
		"data-[state=active]:bg-card data-[state=active]:text-foreground data-[state=active]:shadow-[0_12px_24px_-18px_rgba(15,23,42,0.7)]",
		"data-[state=inactive]:text-muted-foreground hover:data-[state=inactive]:bg-muted/80 hover:data-[state=inactive]:text-foreground",
		"disabled:pointer-events-none disabled:opacity-50",
		className
	)}
	{type}
	{disabled}
	onclick={() => {
		if (!disabled) {
			setValue(value);
		}
	}}
	{...restProps}
>
	{@render children?.()}
</button>
