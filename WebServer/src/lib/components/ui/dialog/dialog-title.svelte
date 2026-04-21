<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLHeadingElement>, HTMLHeadingElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<h2
	bind:this={ref}
	data-slot="dialog-title"
	class={cn("text-2xl font-semibold text-slate-900", className)}
	{...restProps}
>
	{@render children?.()}
</h2>
