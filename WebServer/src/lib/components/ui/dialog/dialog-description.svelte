<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLParagraphElement>, HTMLParagraphElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<p
	bind:this={ref}
	data-slot="dialog-description"
	class={cn("text-sm leading-6 text-slate-600", className)}
	{...restProps}
>
	{@render children?.()}
</p>
