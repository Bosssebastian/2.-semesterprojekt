<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<div
	bind:this={ref}
	data-slot="tabs-list"
	class={cn(
		"bg-background/80 inline-flex h-auto w-fit items-center gap-2 rounded-[1.4rem] border border-border/80 p-1 shadow-sm backdrop-blur-sm",
		className
	)}
	{...restProps}
>
	{@render children?.()}
</div>
